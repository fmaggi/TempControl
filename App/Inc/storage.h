#ifndef _STORAGE_H
#define _STORAGE_H

#include "control.h"

#define CURVE_LENGTH 500
#define NUM_CURVES 3

void Storage_get_curve(uint32_t index, uint16_t curve[CURVE_LENGTH]);
void Storage_set_curve(uint32_t index, const uint16_t curve[CURVE_LENGTH]);

PID Storage_get_PID(void);
void Storage_set_PID(PID pid);

#endif // !_STORAGE_H
