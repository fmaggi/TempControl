#ifndef _CONTROL_H
#define _CONTROL_H

#include <stdint.h>

void Oven_start(void);
void Oven_stop(void);
uint32_t Oven_temperature(void);
void Oven_control(uint32_t current_temperature);
void Oven_set_target(uint32_t temperature);
int32_t Oven_error(void);

#endif
