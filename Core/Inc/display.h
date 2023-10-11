#ifndef __DISPLAY_H
#define __DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "fonts.h"

#define SCREEN_HEIGHT 	240
#define SCREEN_WIDTH 	320

#define BLACK      				0x0000
#define NAVY        			0x000F
#define DARKGREEN   			0x03E0
#define DARKCYAN    			0x03EF
#define MAROON      			0x7800
#define PURPLE      			0x780F
#define OLIVE       			0x7BE0
#define LIGHTGREY   			0xC618
#define DARKGREY    			0x7BEF
#define BLUE        			0x001F
#define GREEN       			0x07E0
#define CYAN        			0x07FF
#define RED         			0xF800
#define MAGENTA     			0xF81F
#define YELLOW      			0xFFE0
#define WHITE       			0xFFFF
#define ORANGE      			0xFD20
#define GREENYELLOW 			0xAFE5
#define PINK        			0xF81F

#define SCREEN_VERTICAL_1		0
#define SCREEN_HORIZONTAL_1		1
#define SCREEN_VERTICAL_2		2
#define SCREEN_HORIZONTAL_2		3

void BSP_Display_init(void);
void BSP_Display_clear(uint16_t color);
void BSP_Display_fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
void BSP_Display_write_text(const char* s, uint16_t x, uint16_t y, const uint8_t* font, uint16_t fg_color, uint16_t bg_color);


#ifdef __cplusplus
}
#endif

#endif /* __DISPLAY_H */
