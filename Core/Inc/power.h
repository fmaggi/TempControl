#ifndef __PWR_H
#define __PWR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void BSP_Power_start(void);
void BSP_Power_stop(void);
void BSP_Power_set(uint32_t power);

void BSP_Power_ZC_interrupt(void);

#ifdef __cplusplus
}
#endif

#endif /* __PWR_H */
