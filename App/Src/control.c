/// TARGET_TEMP:
///     min: 0
///     max: 300
/// CURRENT_TEMP:
///     min: 0
///     max: 300 or else it errors
///
/// ------------------------------
///
/// ERROR (TARGET_TEMP - CURRENT_TEMP):
///     min: 0 - 300 = -300
///     max: 300 - 0 = 300
///
/// -----------------------------
///
/// PID (these values are an upper bound):
///     P: ~max: 20
///     I: ~max: 5
///     D: ~max: 400
///
/// ----------------------------
///
/// proportional term is fine
///     ~max: +/- 6000
///
/// derivative of error can be high if curve has a steep ascent.
/// let's say a curve goes from 0°C to 100°C over 1 second.
/// derivative of error will be 100 as oven will not be able to follow
///     ~max: 40000
///
/// divided by DT which is small, this gets worse
///
/// integral of error can be high if oven doesn't follow curve well.
/// we also limit the max value of integral error to 1000.
///     ~max: 5000
///
/// FP16 -> int16_t
///     max: 32767
///
/// ----------------------------
///
/// Derivative term is the problem
///
/// Conclusion -> Be carefule around derivative term
///

#include "control.h"

#include "bsp.h"
#include "fp16.h"
#include "power.h"
#include "storage.h"
#include "temperature.h"

#include <stdint.h>
#include <string.h>

PID pid = { 0 };

static volatile uint16_t TEMP = 0;
static volatile uint16_t TARGET_TEMP = 0;

#define INTEGRAL_ERROR_LEN 64
#define MAX_INTEGRAL_ERROR 1000

#if MAX_INTEGRAL_ERROR * 5 > 32767
    #error "Integral power component could overflow, reduce MAX_INTEGRAL_ERROR value"
#endif

static volatile int32_t I_ERROR = 0;
static volatile int32_t IE_buf[INTEGRAL_ERROR_LEN] = { 0 };
static volatile uint8_t IE_index = 0;

static volatile int32_t LAST_ERROR = 0;

static volatile int32_t D_ERROR = 0;

static void oven_start(void);
static void oven_stop(void);

static FP16 gradient(struct CurvePoint current, struct CurvePoint next) {
    FP16 dtemp = FP_fromInt((int16_t) (next.temperature - current.temperature));
    uint16_t dtime = next.time_s - current.time_s;
    return dtemp / dtime;
}

void Curve_start(struct CurveRunner* r, uint8_t curve_index) {
    r->index = 0;
    r->ready = 0;
    Storage_get_curve(curve_index, &r->curve);
    struct CurvePoint* points = r->curve.points;
    r->gradient = gradient(points[0], points[1]);
    Oven_set_target(points[0].temperature);
    oven_start();
}

void Curve_stop(struct CurveRunner* r) {
    (void) r;
    oven_stop();
}

static uint16_t get_target(const struct CurveRunner* r, uint16_t time) {
    struct CurvePoint point = r->curve.points[r->index];
    uint16_t dtime = time - point.time_s;
    FP16 dtemp = r->gradient * dtime;
    return point.temperature + (uint16_t) FP_toInt(dtemp);
}

uint8_t Curve_step(struct CurveRunner* r, uint16_t time) {
    if (r->index >= r->curve.length) {
        return 1;
    }

    {
        struct CurvePoint* point = &r->curve.points[r->index + 1];
        if (time >= point->time_s) {
            struct CurvePoint* next = point + 1;
            r->gradient = gradient(*point, *next);
            r->index += 1;
        }
    }

    uint16_t target = get_target(r, time);
    Oven_set_target(target);

    return 0;
}

static void oven_start(void) {
    BSP_T_start();
    BSP_Power_start();
}

PID Oven_get_PID(void) {
    return pid;
}

void Oven_set_PID(PID pid_) {
    pid = pid_;
}

static void oven_stop(void) {
    LAST_ERROR = 0;
    I_ERROR = 0;
    D_ERROR = 0;
    memset((void*) IE_buf, 0, sizeof(IE_buf));
    IE_index = 0;
    Oven_set_target(0);
    BSP_Power_set(0);
    BSP_Power_stop();
    BSP_T_stop();
}

struct Error Oven_error(void) {
    struct Error e;
    e.p = LAST_ERROR;
    e.i = I_ERROR;
    e.d = D_ERROR;
    return e;
}

void Oven_set_temperature(uint16_t temperature) {
    TEMP = temperature;
}

uint16_t Oven_get_temperature(void) {
    return TEMP;
}

uint16_t Oven_get_target(void) {
    return TARGET_TEMP;
}

void Oven_control(uint16_t current_temp) {
    int32_t p_error = (int32_t) TARGET_TEMP - (int32_t) current_temp;

    D_ERROR = p_error - LAST_ERROR;

    I_ERROR -= IE_buf[IE_index];
    I_ERROR += p_error;

    I_ERROR = I_ERROR > MAX_INTEGRAL_ERROR ? MAX_INTEGRAL_ERROR : I_ERROR;
    I_ERROR = I_ERROR < -MAX_INTEGRAL_ERROR ? -MAX_INTEGRAL_ERROR : I_ERROR;

    int32_t total_error = 0;
    total_error += (int32_t) FP_toInt(pid.p * p_error);
    total_error += (int32_t) FP_toInt(pid.i * I_ERROR) * T_SAMPLE_PERIOD_ms / 1000;
    total_error += (int32_t) FP_toInt(pid.d * D_ERROR) * 1000 / T_SAMPLE_PERIOD_ms;

    uint32_t power = total_error > 0 ? (uint32_t) total_error : 0;
    power = power > MAX_POWER ? MAX_POWER : power;

    BSP_Power_set(power);

    LAST_ERROR = p_error;
    IE_buf[IE_index] = p_error;
    IE_index = (IE_index + 1) & (INTEGRAL_ERROR_LEN - 1);
}

void Oven_set_target(uint16_t target) {
    TARGET_TEMP = target;
}
