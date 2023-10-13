#include "temperature.h"

#include "adc.h"
#include "tim.h"

#define ADC_Instance       ADC1

#define ADC_TIM            TIM1
#define ADC_TIM_FREEZE_DBG __HAL_DBGMCU_FREEZE_TIM1

#define FREQ               10000 // Hz
#define TICK               100   // us
#define T_Pin              GPIO_PIN_0
#define T_GPIO_Port        GPIOA

static volatile uint32_t t[T_NUM] = { 0 };

void BSP_T_init() {
    MX_TIM1_Init();
    MX_ADC1_Init();
    ADC_TIM_FREEZE_DBG();

    HAL_ADCEx_Calibration_Start(&hadc1);
    // Calibration enables adc
    HAL_ADC_Stop(&hadc1);
}

void BSP_T_start(void) {
    HAL_TIM_PWM_Start_IT(&htim1, TIM_CHANNEL_1);
    HAL_ADC_Start_IT(&hadc1);
}

void BSP_T_stop(void) {
    HAL_TIM_PWM_Stop_IT(&htim1, TIM_CHANNEL_1);
    HAL_ADC_Stop_IT(&hadc1);
}

volatile uint32_t* BSP_T_get(void) {
    return t;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    for (uint32_t i = 1; i < T_NUM; ++i) {
        t[i] = t[i - 1];
    }
    t[0] = HAL_ADC_GetValue(hadc);
}
