#ifndef __BSP_H
#define __BSP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "display.h"
#include "power.h"
#include "temperature.h"

void BSP_init(void);
void BSP_delay(uint32_t ms);
uint32_t BSP_millis(void);

uint8_t BSP_get_cursor(uint8_t current_cursor);
uint8_t BSP_ok_clicked(void);


#define T_NUM 5
void BSP_start_temp_sensor(void);
void BSP_stop_temp_sensor(void);
volatile uint32_t* BSP_get_temp(void);

void Error(const char* msg);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_H */
