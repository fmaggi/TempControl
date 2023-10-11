#include "display.h"

#include "ILI9341_GFX.h"
#include "ILI9341_STM32_Driver.h"

void BSP_Display_init(void) {
    ILI9341_Init();
}

void BSP_Display_clear(uint16_t color) {
    ILI9341_FillScreen(color);
}

void BSP_Display_fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
    ILI9341_DrawRectangle(x, y, width, height, color);
}

void BSP_Display_write_text(const char* s, uint16_t x, uint16_t y, const uint8_t* font, uint16_t fg_color, uint16_t bg_color) {
    ILI9341_DrawText(s, font, x, y, fg_color, bg_color);
}
