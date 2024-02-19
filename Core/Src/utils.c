#include "utils.h"

#include <stdarg.h>
#include <stdint.h>

int32_t nformat_i32s(char* buf, uint32_t len, const char* fmt, ...) {
    const char* cur = fmt;
    va_list args;
    va_start(args, fmt);

    uint32_t written = 0;
    while (written < len && *cur) {
        if (*cur == '%') {
            int32_t v = va_arg(args, int32_t);
            int32_t w = nformat_i32(buf+written, len-(uint32_t)written, v);
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

    buf[written] = 0;

    return (int32_t)written;

}

int32_t nformat_u32s(char* buf, uint32_t len, const char* fmt, ...) {
    const char* cur = fmt;
    va_list args;
    va_start(args, fmt);

    uint32_t written = 0;
    while (written < len && *cur) {
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

    buf[written] = 0;

    return (int32_t)written;
}

int32_t nformat_u32(char* buf, uint32_t len, uint32_t value) {
    if (value == 0) {
        buf[0] = '0';
        buf[1] = 0;
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

    if (value != 0) {
        // This means that we failed to write the complete number into the buffer
        return -1;
    }

    for (int32_t i = 0; i < written; ++i) {
        buf[i] = aux[written - i - 1];
    }

    buf[written] = 0;

    return written;
}

int32_t nformat_i32(char* buf, uint32_t len, int32_t value) {
    if (value < 0) {
        buf[0] = '-';
        int32_t w = nformat_u32(buf+1, len-1, (uint32_t)-value);
        if (w < 0) {
            return -1;
        }
        return w+1;
    } else {
        return nformat_u32(buf, len, (uint32_t)value);
    }
}
