#ifndef _CONTROL_H
#define _CONTROL_H

#include "fp16.h"

#include <stdint.h>

#define CURVE_LENGTH 500

typedef uint16_t Curve_Point;
typedef Curve_Point Curve[CURVE_LENGTH];
typedef Curve_Point* Curve_Ptr;

typedef union {
    struct {
        FP16 p, i, d;
    };

    FP16 coeffs[3];
} PID;

void Oven_start(void);
void Oven_stop(void);

PID Oven_get_PID(void);
void Oven_set_PID(PID pid);

uint16_t Oven_temperature(void);

void Oven_control(uint16_t current_temperature);
void Oven_set_target(uint16_t temperature);
int32_t Oven_error(void);

#endif
