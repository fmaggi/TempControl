#ifndef __BSP_H
#define __BSP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "display.h"
#include "power.h"
#include "temperature.h"
#include "io.h"

void BSP_init(void);
void BSP_delay(uint32_t ms);
uint32_t BSP_millis(void);

uint8_t BSP_get_cursor(uint8_t current_cursor);
uint8_t BSP_ok_clicked(void);

void Error_Handler(const char* msg);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_H */
