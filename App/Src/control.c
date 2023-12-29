#include "control.h"

#include "bsp.h"
#include "fp16.h"
#include "power.h"
#include "temperature.h"
#include <stdint.h>

PID pid = {0};

static uint16_t temp = 0;
static uint16_t target_temp = 0;

static int32_t last_error = 0;

#define INTEGRAL_ERROR_LEN 64
static int32_t integral_error = 0;
static int32_t ie_buf[INTEGRAL_ERROR_LEN] = {0};
static uint32_t ie_index = 0;

void Oven_start(void) {
    last_error = 0;
    integral_error = 0;
    Oven_set_target(0);
    BSP_Power_set(0);
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
    Oven_set_target(0);
    BSP_Power_stop();
    BSP_T_stop();
}

int32_t Oven_error(void) {
    return last_error;
}

uint16_t Oven_temperature(void) {
    return temp;
}

void Oven_control(uint16_t current_temp) {
    temp = current_temp;

    int32_t error = (int32_t) target_temp - (int32_t) current_temp;

    int32_t d_error = error - last_error;

    integral_error -= ie_buf[ie_index];
    integral_error += error;

    int32_t total_error = 0;
    total_error += (int32_t)pid.p * error;
    total_error += (int32_t)pid.i * integral_error;
    total_error += (int32_t)pid.d * d_error;

    FP16 uv = total_error > 0 ? (FP16)total_error : 0;
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
    Oven_control((uint16_t)temperature);
}
