#include "io.h"

#include "tim.h"

#define IO_TIM TIM4
#define IO_TIM_FREEZE_DBG __HAL_DBGMCU_FREEZE_TIM4

#define Ok_Pin GPIO_PIN_14
#define Ok_GPIO_Port GPIOB
#define Ok_EXTI_IRQn EXTI15_10_IRQn
#define BTN_CLK_Pin GPIO_PIN_6
#define BTN_CLK_GPIO_Port GPIOB
#define BTN_DT_Pin GPIO_PIN_7
#define BTN_DT_GPIO_Port GPIOB

static volatile uint8_t ok_clicked = 0;

void BSP_IO_init(void) {
    MX_TIM4_Init();

    IO_TIM_FREEZE_DBG();

    HAL_TIM_Encoder_Start_IT(&htim4, TIM_CHANNEL_ALL);
}

uint8_t BSP_IO_get_cursor(uint8_t current_cursor) {
    (void) current_cursor;
    return (uint8_t) IO_TIM->CNT;
}

void BSP_IO_Ok_interrupt(void) {
    ok_clicked = 1;
}

uint8_t BSP_IO_ok_clicked(void) {
    uint8_t temp = ok_clicked;
    ok_clicked = 0;
    return temp;
}
