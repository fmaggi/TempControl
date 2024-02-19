#ifndef __PWR_H
#define __PWR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define MAX_POWER 100

struct PowerState {
    uint32_t power;
    uint8_t on;
};

void BSP_Power_init(void);
void BSP_Power_start(void);
void BSP_Power_stop(void);

struct PowerState BSP_Power_get(void);
void BSP_Power_delta(int32_t d);
void BSP_Power_set(uint32_t power);

void BSP_Power_ZC_interrupt(void);

#ifdef __cplusplus
}
#endif

#endif /* __PWR_H */
