#ifndef _UTILS_H
#define _UTILS_H

#include <stdint.h>

int32_t nformat_u32s(char* buf, uint32_t len, const char* fmt, ...);
int32_t nformat_i32s(char* buf, uint32_t len, const char* fmt, ...);

int32_t nformat_u32(char *buf, uint32_t len, uint32_t value);
int32_t nformat_i32(char *buf, uint32_t len, int32_t value);

#endif // !_UTILS_H
