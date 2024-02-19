#ifndef _FP16_H
#define _FP16_H

#include "bsp.h"
#include "utils.h"

typedef int32_t FP16;

#define SHIFT  16
#define SCALE  (1 << SHIFT)
#define MASK   (SCALE - 1)
#define ONE    ((FP16) (1 << SHIFT))
#define MAX    ((FP16) ((uint32_t) ~0 >> 1))
#define MIN    (~MAX)
#define ABS(a) ((a) < 0 ? (-(a)) : (a))

static inline FP16 FP_fromInt(int16_t n) {
    return (FP16) ((int32_t) n << SHIFT);
}

static inline int16_t FP_toInt(FP16 fp) {
    return (int16_t) (fp >> SHIFT);
}

static inline FP16 FP_mul(FP16 a, FP16 b) {
    return (a * b) >> SHIFT;
}

/// caller garantees b != 0
static inline FP16 FP_div(FP16 a, FP16 b) {
    if (ABS(b) <= ONE) {
        if (a < 0 && b < 0) {
            if ((int64_t) a << SHIFT < (int64_t) MAX * (int64_t) b) {
                // overflow
                return MAX;
            }
        } else if (b > 0) {
            if ((int64_t) a << SHIFT > (int64_t) MAX * (int64_t) b) {
                // overflow
                return MAX;
            }
            if ((int64_t) a << SHIFT < (int64_t) MIN * (int64_t) b) {
                // underflow
                return MIN;
            }
        } else if (b < 0) {
            if ((int64_t) a << SHIFT > (int64_t) MIN * (int64_t) b) {
                // underflow
                return MIN;
            }
        }
    }
    return (FP16) ((int64_t) a << SHIFT) / b;
}

static inline int16_t FP_decimal(FP16 a) {
    return FP_toInt(a);
}

static inline int16_t FP_frac(FP16 a) {
    return (int16_t) (a & MASK);
}

static inline int16_t FP_frac_as_int(FP16 a, uint32_t precision) {
    return (int16_t) ((((int64_t) FP_frac(a)) * precision) >> SHIFT);
}

static inline void FP_format(char* buf, uint32_t len, FP16 a, uint32_t precision) {
    const int16_t d = FP_decimal(a);
    const int16_t f = FP_frac(a);
    const int16_t fi = FP_frac_as_int(a, precision);
    if (f >= ONE / 10) {
        nformat_i32s(buf, len, "%.%", d, fi);
    } else if (f >= ONE / 100) {
        nformat_i32s(buf, len, "%.0%", d, fi);
    } else {
        nformat_i32s(buf, len, "%.00", d);
    }
}

#endif
