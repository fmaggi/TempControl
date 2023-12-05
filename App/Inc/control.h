#ifndef _CONTROL_H
#define _CONTROL_H

#include <stdint.h>

typedef union {
    struct {
        uint32_t p, i, d;
    };
    uint32_t coeffs[3];
} PID;

void Oven_start(void);
void Oven_stop(void);
PID Oven_get_PID(void);
void Oven_set_PID(PID pid);
uint32_t Oven_temperature(void);
void Oven_control(uint32_t current_temperature);
void Oven_set_target(uint32_t temperature);
int32_t Oven_error(void);

#endif
