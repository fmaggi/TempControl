#ifndef __T_H
#define __T_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define T_NUM 5
void BSP_start_temp_sensor(void);
void BSP_stop_temp_sensor(void);
volatile uint32_t* BSP_get_temp(void);

#ifdef __cplusplus
}
#endif

#endif /* __T_H */
