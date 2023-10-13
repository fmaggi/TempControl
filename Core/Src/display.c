#include "display.h"

#include "ILI9341_GFX.h"
#include "ILI9341_STM32_Driver.h"
#include "spi.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_spi.h"

void Error(const char* msg);

SPI_HandleTypeDef hspi1;
#define SPI_Instance SPI1

#define LCD_LED_Pin       GPIO_PIN_0
#define LCD_LED_GPIO_Port GPIOB
#define LCD_RST_Pin       GPIO_PIN_1
#define LCD_RST_GPIO_Port GPIOB
#define LCD_DC_Pin        GPIO_PIN_10
#define LCD_DC_GPIO_Port  GPIOB
#define LCD_CS_Pin        GPIO_PIN_11
#define LCD_CS_GPIO_Port  GPIOB

static void GPIO_init(void);
static void SPI_init(void);

void BSP_Display_init(void) {
    GPIO_init();
    SPI_init();
    /* MX_SPI1_Init(); */
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

static void GPIO_init(void) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    HAL_GPIO_WritePin(GPIOB, LCD_LED_Pin | LCD_RST_Pin | LCD_DC_Pin | LCD_CS_Pin, GPIO_PIN_RESET);
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    GPIO_InitStruct.Pin = LCD_LED_Pin | LCD_RST_Pin | LCD_DC_Pin | LCD_CS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

static void SPI_init(void) {
    hspi1.Instance = SPI_Instance;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 10;
    if (HAL_SPI_Init(&hspi1) != HAL_OK) {
        Error_Handler();
    }
}
