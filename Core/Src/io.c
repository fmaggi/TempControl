#include "io.h"

#include "bsp.h"
#include "bsp_internal.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_def.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_uart.h"

TIM_HandleTypeDef htim4;
#define IO_TIM            TIM4
#define IO_TIM_FREEZE_DBG __HAL_DBGMCU_FREEZE_TIM4

#define DEBOUNCE 50

UART_HandleTypeDef huart1;

static volatile uint8_t ok_clicked = 0;
static volatile uint8_t received = 0;

static void GPIO_init(void);
static void TIM_init(void);
static void USART_init(void);

void BSP_IO_init(void) {
    GPIO_init();
    TIM_init();
    USART_init();

    IO_TIM_FREEZE_DBG();

    HAL_TIM_Encoder_Start_IT(&htim4, TIM_CHANNEL_ALL);
}

uint8_t BSP_IO_get_cursor(uint8_t current_cursor, uint8_t num_positions) {
    (void) current_cursor;
    return (uint8_t) ((IO_TIM->CNT >> 2) % num_positions);
}

void BSP_IO_Ok_interrupt(void) {
    static uint32_t last_t = 0;
    uint32_t t = HAL_GetTick();
    if (t > last_t + DEBOUNCE) {
        last_t = t;
        ok_clicked = 1;
    }
}

uint8_t BSP_IO_ok_clicked(void) {
    uint8_t temp = ok_clicked;
    ok_clicked = 0;
    return temp;
}

uint32_t BSP_IO_get_rotary(uint32_t min_value, uint32_t max_value) {
    uint32_t cnt = IO_TIM->CNT >> 2;
    if (min_value > 0 && cnt <= min_value) {
        IO_TIM->CNT = min_value << 2;
        return min_value;
    }

    if (cnt >= max_value) {
        IO_TIM->CNT = max_value << 2;
        return max_value;
    }

    return cnt;
}

void BSP_IO_set_rotary(uint32_t v) {
    IO_TIM->CNT = v << 2;
}

void BSP_IO_Toggle_LED(void) {
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
}

void BSP_Comms_abort(void) {
    HAL_UART_Abort(&huart1);
}

void BSP_Comms_transmit_block(uint8_t* buf, uint16_t size) {
    HAL_UART_Transmit(&huart1, buf, size, HAL_MAX_DELAY);
}

void BSP_Comms_transmit(uint8_t* buf, uint16_t size) {
    HAL_UART_Transmit_IT(&huart1, buf, size);
}

void BSP_Comms_receive_block(uint8_t* buf, uint16_t size) {
    const HAL_StatusTypeDef e = HAL_UART_Receive(&huart1, buf, size, HAL_MAX_DELAY - 1);
    switch (e) {
        case HAL_OK: return;
        case HAL_ERROR: Error_Handler("UART Error");
        case HAL_BUSY: Error_Handler("UART Busy");
        case HAL_TIMEOUT: Error_Handler("UART Timeout");
    }
}

void BSP_Comms_receive_expect(uint8_t* buf, uint16_t size) {
    HAL_UART_Receive_IT(&huart1, buf, size);
}

uint8_t BSP_Comms_received(void) {
    uint8_t temp = received;
    received = 0;
    return temp;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart) {
    (void) huart;
    received = 1;
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

    HAL_NVIC_SetPriority(Ok_EXTI_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(Ok_EXTI_IRQn);
}

static void TIM_init(void) {
    TIM_Encoder_InitTypeDef sConfig = { 0 };
    TIM_MasterConfigTypeDef sMasterConfig = { 0 };

    htim4.Instance = IO_TIM;
    htim4.Init.Prescaler = 0;
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.Period = 0xFFFF;
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
        Error_Handler("IO: Failed to init TIM");
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK) {
        Error_Handler("IO: Failed to init TIM");
    }
}

static void USART_init(void) {
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 9600;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK) {
        Error_Handler("IO: Failed to init USART");
    }
}
