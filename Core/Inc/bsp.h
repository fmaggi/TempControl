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

#define MOD_POW2(a, b) (a) & ((b)-1)

#define FLASH_STORAGE __attribute__((__section__(".user_data")))

void BSP_init(void);
void BSP_delay(uint32_t ms);
uint32_t BSP_millis(void);

void BSP_Flash_write(void* address_start, uint32_t numberofwords, uint32_t* data);

void Error_Handler(const char* msg);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_H */
