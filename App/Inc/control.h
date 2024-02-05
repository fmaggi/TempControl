#ifndef _CONTROL_H
#define _CONTROL_H

#include "fp16.h"

#include <stdint.h>

#define T_SAMPLE_PERIOD_ms 100
#define MAX_CURVE_LENGTH 50

struct CurvePoint {
    uint16_t time_s;
    uint16_t temperature;
};

struct Curve {
    struct CurvePoint points[MAX_CURVE_LENGTH];
    uint8_t length;
};

struct CurveRunner {
    struct Curve curve;
    FP16 gradient;
    uint8_t index;
    uint8_t ready; // ready determines stabilization around first point
};

void Curve_start(struct CurveRunner* curve, uint8_t curve_index);
uint8_t Curve_step(struct CurveRunner* curve, uint16_t time);

typedef union {
    struct {
        FP16 p, i, d;
    };

    FP16 coeffs[3];
} PID;

PID Oven_get_PID(void);
void Oven_set_PID(PID pid);

uint16_t Oven_target(void);
uint16_t Oven_temperature(void);

void Oven_control(uint16_t current_temperature);
void Oven_set_target(uint16_t temperature);

struct Error {
    int32_t p, i, d;
};

struct Error Oven_error(void);

#endif
