#ifndef _CONTROL_H
#define _CONTROL_H

#include "fp16.h"

#include <stdint.h>

#define CURVE_LENGTH 50

typedef struct {
    uint16_t time_s;
    uint16_t temperature;
} CurvePoint;

typedef struct {
    CurvePoint points[CURVE_LENGTH];
    FP16 gradient;
    uint8_t index;
} Curve;

// curve points must already be set
void Curve_start(Curve* curve);
uint16_t Curve_target(const Curve* curve, uint16_t time);
uint8_t Curve_step(Curve* curve, uint16_t time);

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

struct Error {
    int32_t p, i, d;
};

struct Error Oven_error(void);

#endif
