#ifndef __IO_H
#define __IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void BSP_IO_init(void);
uint8_t BSP_IO_get_cursor(uint8_t current_cursor);
void BSP_IO_Ok_interrupt(void);
uint8_t BSP_IO_ok_clicked(void);
uint32_t BSP_IO_get_rotary(uint32_t min_value, uint32_t max_value);

#ifdef __cplusplus
}
#endif

#endif /* __IO_H */
