#include "temperature.h"

#include "bsp_internal.h"
#include "tim.h"
#include "adc.h"

static volatile uint32_t t[T_NUM] = {0};

void BSP_start_temp_sensor(void) {
    HAL_TIM_PWM_Start_IT(&htim1, TIM_CHANNEL_1);
    HAL_ADC_Start_IT(&hadc1);
    __HAL_DBGMCU_FREEZE_TIM1();
}

void BSP_stop_temp_sensor(void) {
    HAL_TIM_PWM_Stop_IT(&htim1, TIM_CHANNEL_1);
    HAL_ADC_Stop_IT(&hadc1);
}

volatile uint32_t* BSP_get_temp(void) {
    return t;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    for (uint32_t i = 1; i < T_NUM; ++i) {
        t[i] = t[i-1];        
    }
    t[0] = HAL_ADC_GetValue(hadc);
}
