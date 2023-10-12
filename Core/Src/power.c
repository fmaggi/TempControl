#include "power.h"

#include "bsp_internal.h"
#include "tim.h"

#define TICK               250      // ps
#define HALF_CYCLE_TIME    10000000 // ps. Duration of a half cycle of the power network at 50Hz
#define MAX_POWER          4096     // This is due to ADC conversion going from 0 to 4096
#define UNREACHABLE_PERIOD 0xFFFF
static const uint32_t MAX_PERIOD = HALF_CYCLE_TIME / TICK;
static const uint32_t POWER_TO_PERIOD = MAX_PERIOD / MAX_POWER;
static const uint32_t HALF_POWER = MAX_POWER / 2;

static volatile uint32_t period1 = UNREACHABLE_PERIOD;
static volatile uint32_t period2 = UNREACHABLE_PERIOD;

void BSP_start_power_step(void) {
    HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_1);
    HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_2);
    HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_3);
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);
}

void BSP_stop_power_step(void) {
    HAL_NVIC_DisableIRQ(EXTI1_IRQn);
    HAL_TIM_OC_Stop_IT(&htim3, TIM_CHANNEL_1);
    HAL_TIM_OC_Stop_IT(&htim3, TIM_CHANNEL_2);
    HAL_TIM_OC_Stop_IT(&htim3, TIM_CHANNEL_3);
}

void BSP_set_power(uint32_t power) {
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
    TIM3->CNT = 0x0;
    TIM3->CCR1 = period1;
    TIM3->CCR2 = period2;
    __enable_irq();
}
