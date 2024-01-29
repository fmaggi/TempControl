#ifndef _FP16_H
#define _FP16_H

#include "utils.h"

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

static inline uint16_t FP_frac(FP16 a) {
    return (uint16_t) (a & MASK);
}

static inline uint16_t FP_frac_as_int(FP16 a, uint32_t precision) {
    return (uint16_t) ((((uint64_t) FP_frac(a)) * precision) >> SHIFT);
}

static inline void FP_format(char* buf, uint32_t len, FP16 a, uint32_t precision) {
    const uint16_t d = FP_decimal(a);
    const uint16_t f = FP_frac(a);
    const uint16_t fi = FP_frac_as_int(a, precision);
    if (f >= ONE / 10) {
        nformat_u32s(buf, len, "%.%", d, fi);
    } else if (f >= ONE / 100) {
        nformat_u32s(buf, len, "%.0%", d, fi);
    } else {
        nformat_u32s(buf, len, "%.00", d);
    }
}

#endif
