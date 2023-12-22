#ifndef __IO_H
#define __IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void BSP_IO_init(void);
uint8_t BSP_IO_get_cursor(uint8_t current_cursor, uint8_t num_positions);
void BSP_IO_Ok_interrupt(void);
uint8_t BSP_IO_ok_clicked(void);
uint32_t BSP_IO_get_rotary(uint32_t min_value, uint32_t max_value);
void BSP_IO_set_rotary(uint32_t value);
void BSP_IO_Toggle_LED(void);

void BSP_Comms_abort(void);
void BSP_Comms_transmit(uint8_t* buf, uint16_t size);
void BSP_Comms_transmit_block(uint8_t* buf, uint16_t size);
void BSP_Comms_receive_block(uint8_t* buf, uint16_t size);
void BSP_Comms_receive_expect(uint8_t* buf, uint16_t size);
uint8_t BSP_Comms_received(void);

#ifdef __cplusplus
}
#endif

#endif /* __IO_H */
