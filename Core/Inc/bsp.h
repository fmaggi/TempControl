#ifndef __BSP_H
#define __BSP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "display.h"

void BSP_Init(void);
void BSP_Delay(uint32_t ms);

uint8_t BSP_get_cursor(uint8_t current_cursor);
uint8_t BSP_ok_clicked(void);

void BSP_start_temp_sensor(void);
uint32_t BSP_get_temp(void);

void BSP_start_power_step(void);
void BSP_set_power(uint32_t power);

void Error(const char* msg);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_H */
