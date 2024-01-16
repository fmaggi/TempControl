#ifndef _STORAGE_H
#define _STORAGE_H

#include "control.h"

#define NUM_CURVES 3

void Storage_get_curve(uint32_t index, Curve* curve);
void Storage_set_curve(uint32_t index, const Curve* curve);

PID Storage_get_PID(void);
void Storage_set_PID(PID pid);

#endif // !_STORAGE_H
