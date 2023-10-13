#include "power.h"

#include "bsp_internal.h"
#include "stm32f1xx_hal_rcc.h"

TIM_HandleTypeDef htim3;
#define P_TIM          TIM3
#define TIM_FREEZE_DBG __HAL_DBGMCU_FREEZE_TIM3

#define FREQ               4000000
#define TICK               250      // ps
#define HALF_CYCLE_TIME    10000000 // ps. Duration of a half cycle of the power network at 50Hz
#define MAX_POWER          4096     // This is due to ADC conversion going from 0 to 4096
#define UNREACHABLE_PERIOD 0xFFFF
static const uint32_t MAX_PERIOD = HALF_CYCLE_TIME / TICK;
static const uint32_t POWER_TO_PERIOD = MAX_PERIOD / MAX_POWER;
static const uint32_t HALF_POWER = MAX_POWER / 2;

static volatile uint32_t period1 = UNREACHABLE_PERIOD;
static volatile uint32_t period2 = UNREACHABLE_PERIOD;

static void TIM_init(void);
static void GPIO_init(void);

void BSP_Power_init(void) {
    GPIO_init();
    TIM_init();
    TIM_FREEZE_DBG();
}

void BSP_Power_start(void) {
    HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_1);
    HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_2);
    HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_3);
    HAL_NVIC_EnableIRQ(ZC_EXTI_IRQn);
}

void BSP_Power_stop(void) {
    HAL_NVIC_DisableIRQ(ZC_EXTI_IRQn);
    HAL_TIM_OC_Stop_IT(&htim3, TIM_CHANNEL_1);
    HAL_TIM_OC_Stop_IT(&htim3, TIM_CHANNEL_2);
    HAL_TIM_OC_Stop_IT(&htim3, TIM_CHANNEL_3);
}

void BSP_Power_set(uint32_t power) {
    {
        uint32_t power1 = power > HALF_POWER ? MAX_POWER : power * 2;
        period1 = (MAX_POWER - power1) * POWER_TO_PERIOD;
    }

    {
        uint32_t power2 = power >= HALF_POWER ? (power - HALF_POWER) * 2 : 0;
        period2 = (MAX_POWER - power2) * POWER_TO_PERIOD;
    }
}

void BSP_Power_ZC_interrupt(void) {
    __disable_irq();
    P_TIM->CNT = 0x0;
    P_TIM->CCR1 = period1;
    P_TIM->CCR2 = period2;
    __enable_irq();
}

void Trigger_Triac(GPIO_TypeDef* port, uint16_t pin) {
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);

    // HACK: this should be around 5 us at 8 MHz (measured it)
    for (int i = 0; i < 5; ++i)
        ;

    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef* htim) {
    __disable_irq();
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
        /* htim->Instance->CCR1 = UNREACHABLE_PERIOD; */
        Trigger_Triac(Triac1_GPIO_Port, Triac1_Pin);
    } else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
        /* htim->Instance->CCR2 = UNREACHABLE_PERIOD; */
        Trigger_Triac(Triac2_GPIO_Port, Triac2_Pin);
    } else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3) {
        // If we get here it's an error
        htim->Instance->CNT = 0x0;
        HAL_TIM_OC_Stop_IT(&htim3, TIM_CHANNEL_1);
        HAL_TIM_OC_Stop_IT(&htim3, TIM_CHANNEL_2);
        HAL_TIM_OC_Stop_IT(&htim3, TIM_CHANNEL_3);

        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
        Error_Handler("404: 220V not found!");
    } else {
        __NOP();
    }
    __enable_irq();
}

static void TIM_init(void) {
    TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
    TIM_MasterConfigTypeDef sMasterConfig = { 0 };
    TIM_OC_InitTypeDef sConfigOC = { 0 };

    uint32_t prescaler = HAL_RCC_GetSysClockFreq() / FREQ;

    htim3.Instance = P_TIM;
    htim3.Init.Prescaler = prescaler - 1;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 0xffff;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim3) != HAL_OK) {
        Error_Handler("PWR: Failed to init TIM");
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK) {
        Error_Handler("PWR: Failed to init TIM");
    }
    if (HAL_TIM_OC_Init(&htim3) != HAL_OK) {
        Error_Handler("PWR: Failed to init TIM");
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK) {
        Error_Handler("PWR: Failed to init TIM");
    }
    sConfigOC.OCMode = TIM_OCMODE_TIMING;
    sConfigOC.Pulse = 0xffff;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_OC_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) {
        Error_Handler("PWR: Failed to init TIM");
    }
    if (HAL_TIM_OC_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) {
        Error_Handler("PWR: Failed to init TIM");
    }
    sConfigOC.Pulse = 0xfff0;
    if (HAL_TIM_OC_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK) {
        Error_Handler("PWR: Failed to init TIM");
    }
}

static void GPIO_init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    __HAL_RCC_GPIOA_CLK_ENABLE();

    HAL_GPIO_WritePin(GPIOA, Triac1_Pin | Triac2_Pin, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = ZC_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ZC_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pins : PAPin PAPin */
    GPIO_InitStruct.Pin = Triac1_Pin | Triac2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(ZC_EXTI_IRQn, 0, 0);
}
