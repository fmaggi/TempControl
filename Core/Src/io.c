#include "io.h"

#include "tim.h"

TIM_HandleTypeDef htim4;
#define IO_TIM            TIM4
#define IO_TIM_FREEZE_DBG __HAL_DBGMCU_FREEZE_TIM4

#define Ok_Pin            GPIO_PIN_14
#define Ok_GPIO_Port      GPIOB
#define Ok_EXTI_IRQn      EXTI15_10_IRQn
#define BTN_CLK_Pin       GPIO_PIN_6
#define BTN_CLK_GPIO_Port GPIOB
#define BTN_DT_Pin        GPIO_PIN_7
#define BTN_DT_GPIO_Port  GPIOB

static volatile uint8_t ok_clicked = 0;

static void GPIO_init(void);
static void TIM_init(void);

void BSP_IO_init(void) {
    GPIO_init();
    TIM_init();
    /* MX_TIM4_Init(); */

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

static void GPIO_init(void) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    GPIO_InitStruct.Pin = LED_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = Ok_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(Ok_GPIO_Port, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

static void TIM_init(void) {
    TIM_Encoder_InitTypeDef sConfig = { 0 };
    TIM_MasterConfigTypeDef sMasterConfig = { 0 };

    /* USER CODE BEGIN TIM4_Init 1 */

    /* USER CODE END TIM4_Init 1 */
    htim4.Instance = IO_TIM;
    htim4.Init.Prescaler = 0;
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.Period = 2;
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
    sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
    sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
    sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
    sConfig.IC1Filter = 0;
    sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
    sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
    sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
    sConfig.IC2Filter = 0;
    if (HAL_TIM_Encoder_Init(&htim4, &sConfig) != HAL_OK) {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK) {
        Error_Handler();
    }
}
