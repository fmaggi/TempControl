#include "utils.h"

#include <stdarg.h>

int32_t nformat_u32s(char* buf, uint32_t len, const char* fmt, ...) {
    const char* cur = fmt;
    va_list args;
    va_start(args, fmt);

    uint32_t written = 0;
    while ((uint32_t)written < len && *cur) {
        if (*cur == '%') {
            uint32_t v = va_arg(args, uint32_t);
            int32_t w = nformat_u32(buf+written, len-(uint32_t)written, v);
            if (w == -1) {
                return -1;
            }
            written += (uint32_t)w;
        } else {
            buf[written] = *cur;
            written += 1;
        }

        cur++;
    }

    for (uint32_t i = written; i < len; ++i) {
        buf[i] = 0;
    }

    return (int32_t)written;
}

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

    if (len < (uint32_t)written) {
        return -1;
    }

    for (int32_t i = 0; i < written; ++i) {
        buf[i] = aux[written - i - 1];
    }

    return written;
}
