#include "control.h"

#include "bsp.h"
#include "fp16.h"
#include "power.h"
#include "storage.h"
#include "temperature.h"

#include <stdint.h>
#include <string.h>

PID pid = { 0 };
static const volatile FP16 dt = (ONE * T_SAMPLE_PERIOD_ms) / 1000;

static volatile uint16_t temp = 0;
static volatile uint16_t target_temp = 0;

#define MAX_INITIAL_ERROR  3
#define INTEGRAL_ERROR_LEN 16
#define MAX_INTEGRAL_ERROR 1000

static volatile int32_t i_error = 0;
static volatile int32_t ie_buf[INTEGRAL_ERROR_LEN] = { 0 };
static volatile uint8_t ie_index = 0;

static volatile int32_t last_error = 0;

static volatile int32_t d_error = 0;

static void oven_start(void);
static void oven_stop(void);

static void update_gradient(CurveRunner* r) {
    CurvePoint current = r->curve.points[r->index];
    CurvePoint next = r->curve.points[r->index + 1];

    FP16 dtemp = FP_fromInt(next.temperature - current.temperature);
    uint16_t dtime = next.time_s - current.time_s;
    r->gradient = dtemp / dtime;
}

void Curve_start(CurveRunner* r, uint8_t curve_index) {
    r->index = 0;
    r->ready = 0;
    Storage_get_curve(curve_index, &r->curve);
    update_gradient(r);
    Oven_set_target(r->curve.points[0].temperature);
    oven_start();
}

static void stabilize(CurveRunner* r) {
    uint16_t error;
    if (temp < target_temp) {
        error = target_temp - temp;
    } else {
        error = temp - target_temp;
    }

    if (error > MAX_INITIAL_ERROR) {
        return;
    }

    const uint16_t ierror = i_error < 0 ? (uint16_t) -i_error : (uint16_t) i_error;
    if (ierror > MAX_INITIAL_ERROR) {
        return;
    }

    r->ready = 1;
}

uint16_t Curve_target(const CurveRunner* r, uint16_t time) {
    CurvePoint point = r->curve.points[r->index];
    uint16_t dt = time - point.time_s;
    FP16 dT = r->gradient * dt;
    return point.temperature + FP_toInt(dT);
}

CurveState Curve_step(CurveRunner* r, uint16_t time) {
    if (!r->ready) {
        stabilize(r);
        return PREP;
    }

    if (r->index >= r->curve.length) {
        oven_stop();
        return DONE;
    }

    CurvePoint point = r->curve.points[r->index + 1];
    if (time >= point.time_s) {
        r->index += 1;
        update_gradient(r);
    }

    uint16_t target = Curve_target(r, time);
    Oven_set_target(target);

    return RUN;
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
    last_error = 0;
    i_error = 0;
    d_error = 0;
    memset((void*)ie_buf, 0, sizeof(ie_buf));
    ie_index = 0;
    Oven_set_target(0);
    BSP_Power_set(0);
    BSP_Power_stop();
    BSP_T_stop();
}

struct Error Oven_error(void) {
    struct Error e;
    e.p = last_error;
    e.i = i_error;
    e.d = d_error;
    return e;
}

uint16_t Oven_temperature(void) {
    return temp;
}

uint16_t Oven_target(void) {
    return target_temp;
}

void Oven_control(uint16_t current_temp) {
    temp = current_temp;

    int32_t error = (int32_t) target_temp - (int32_t) current_temp;

    d_error = error - last_error;

    i_error -= ie_buf[ie_index];
    i_error += error;

    i_error = i_error > MAX_INTEGRAL_ERROR ? MAX_INTEGRAL_ERROR : i_error;
    i_error = i_error < -MAX_INTEGRAL_ERROR ? -MAX_INTEGRAL_ERROR : i_error;

    int32_t total_error = 0;
    total_error += (int32_t) pid.p * error;
    total_error += (int32_t) pid.i * i_error * (int32_t)dt;
    total_error += (int32_t) pid.d * d_error / (int32_t)dt;

    FP16 uv = total_error > 0 ? (FP16) total_error : 0;
    uint32_t power = FP_toInt(uv);
    power = power > MAX_POWER ? MAX_POWER : power;

    BSP_Power_set(power);

    last_error = error;
    ie_buf[ie_index] = error;
    ie_index = (ie_index + 1) & (INTEGRAL_ERROR_LEN - 1);
}

void Oven_set_target(uint16_t target) {
    target_temp = target;
}

void BSP_T_on_conversion(uint32_t temperature) {
    if (temperature > UINT16_MAX) {
        Error_Handler("Invalid temperature");
    }
    Oven_control((uint16_t) temperature);
}
