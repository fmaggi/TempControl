#ifndef _FP16_H
#define _FP16_H

#include "utils.h"
#include <stdint.h>
#include <stdio.h>

typedef uint32_t FP16;

#define SHIFT 16
#define SCALE (1 << SHIFT)
#define MASK  (SCALE - 1)
#define ONE   ((FP16) (1 << SHIFT))

static inline FP16 FP_fromInt(uint16_t n) {
    return (FP16) ((uint32_t) n << SHIFT);
}

static inline uint16_t FP_toInt(FP16 fp) {
    return (uint16_t) (fp >> SHIFT);
}

static inline FP16 FP_mul(FP16 a, FP16 b) {
    return (a * b) >> SHIFT;
}

static inline FP16 FP_div(FP16 a, FP16 b) {
    return (FP16) ((uint64_t) a << SHIFT) / b;
}

static inline uint16_t FP_decimal(FP16 a) {
    return FP_toInt(a);
}

static inline uint16_t FP_frac(FP16 a, uint32_t precision) {
    return (uint16_t) ((((uint64_t) a & MASK) * precision) >> SHIFT);
}

static inline void FP_format(char* buf, FP16 a, uint32_t precision) {
    const uint16_t d = FP_decimal(a);
    const uint16_t f = FP_frac(a, precision);
    if ((a & MASK) < ONE / 100) {
        int32_t w = nformat_u32(buf, 50, d);
        buf[w] = '.';
        buf[w+1] = '0';
        buf[w+2] = '0';
    } else if ((a & MASK) < ONE / 10) {
        int32_t w = nformat_u32(buf, 50, d);
        buf[w] = '.';
        buf[w+1] = '0';
        nformat_u32(buf, 50-((uint32_t)w+2), f);
    } else {
        int32_t w = nformat_u32(buf, 50, d);
        buf[w] = '.';
        nformat_u32(buf, 50-((uint32_t)w+1), f);
    }
}

#endif
