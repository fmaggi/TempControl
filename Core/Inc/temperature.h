#ifndef __T_H
#define __T_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define T_NUM 5
void BSP_T_init();
void BSP_T_start(void);
void BSP_T_stop(void);
volatile uint32_t* BSP_T_get(void);

#ifdef __cplusplus
}
#endif

#endif /* __T_H */
