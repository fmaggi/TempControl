#ifndef __T_H
#define __T_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void BSP_T_init(uint32_t sample_period_us);
void BSP_T_start(void);
void BSP_T_stop(void);

// Implemented by user

void BSP_T_on_conversion(uint32_t temperature);

#ifdef __cplusplus
}
#endif

#endif /* __T_H */
