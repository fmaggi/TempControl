#include "utils.h"

#ifndef __ZIG__

int32_t nformat_u32(char* buf, uint32_t len, uint32_t value) {
    if (value == 0) {
        buf[0] = '0';
        return 1;
    }

    int32_t written = 0;

    // max value = 4294967296
    char aux[10];
    const uint32_t max = len < 10 ? len : 10;
    for (uint32_t i = 0; i < max; ++i) {
        if (value == 0) break;
        aux[i] = '0' + (value % 10);
        value /= 10;
        written += 1;
    }

    for (int32_t i = 0; i < written; ++i) {
        buf[i] = aux[written - i - 1];
    }

    return written;
}

#endif
