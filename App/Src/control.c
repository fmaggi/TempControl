#include "control.h"

#include "bsp.h"
#include "power.h"
#include "temperature.h"

PID pid = {{ 10, 0, 140 }};

static uint32_t temp = 0;
static uint32_t target_temp = 0;

static int32_t last_error = 0;
static int32_t integral_error = 0;

void Oven_start(void) {
    BSP_T_start();
    BSP_Power_start();
}

PID Oven_get_PID(void) {
    return pid;
}

void Oven_set_PID(PID pid_) {
    pid = pid_;
}

void Oven_stop(void) {
    BSP_Power_stop();
    BSP_T_stop();
}

int32_t Oven_error(void) {
    return last_error;
}

uint32_t Oven_temperature(void) {
    return temp;
}

void Oven_control(uint32_t current_temp) {
    temp = current_temp;
    int32_t error = (int32_t) target_temp - (int32_t) current_temp;
    int32_t d_error = error - last_error;

    int32_t v = (int32_t)pid.p * error + (int32_t)pid.i * integral_error + (int32_t)pid.d * d_error;

    uint32_t power = v > 0 ? (uint32_t)v : 0;
    power = power > MAX_POWER ? MAX_POWER : power;

    BSP_Power_set(power);

    last_error = error;
    integral_error += error;
}

void Oven_set_target(uint32_t target) {
    target_temp = target;
}

void BSP_T_on_conversion(uint32_t temperature) {
    Oven_control(temperature);
}
