#include "temperature.h"

#include "bsp_internal.h"
#include "stm32f1xx_hal_adc.h"
#include "stm32f1xx_hal_rcc.h"

ADC_HandleTypeDef hadc1;
#define ADC_Instance ADC1

TIM_HandleTypeDef htim1;
#define ADC_TIM            TIM1
#define ADC_TIM_FREEZE_DBG __HAL_DBGMCU_FREEZE_TIM1

#define FREQ        10000 // Hz
#define TICK        100   // us
#define T_Pin       GPIO_PIN_0
#define T_GPIO_Port GPIOA

#define MAX_V 3300000 // uV
#define SAMPLES 4096

static const uint32_t msb = MAX_V / SAMPLES; // uV

static volatile uint32_t t[T_NUM] = { 0 };

static void TIM_init(uint32_t sample_period);
static void ADC_init(void);

void BSP_T_init(uint32_t sample_period_us) {
    TIM_init(sample_period_us);
    ADC_init();
    ADC_TIM_FREEZE_DBG();

    // Calibration enables adc
    HAL_ADC_Stop(&hadc1);
}

void BSP_T_start(void) {
    HAL_ADCEx_Calibration_Start(&hadc1);
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
    const uint32_t conv = HAL_ADC_GetValue(hadc);
    const uint32_t uV = conv * msb;
    const uint32_t mV = uV / 1000;
    t[0] = (mV - 54) / 10;
}

static void TIM_init(uint32_t sample_period) {
    TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
    TIM_MasterConfigTypeDef sMasterConfig = { 0 };
    TIM_OC_InitTypeDef sConfigOC = { 0 };
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = { 0 };

    if (sample_period < TICK) {
        Error_Handler("T: Failed to init TIM");
    }

    uint32_t prescaler = HAL_RCC_GetSysClockFreq() / FREQ;
    uint32_t period = sample_period / TICK;

    htim1.Instance = ADC_TIM;
    htim1.Init.Prescaler = prescaler - 1;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = period + 1;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim1) != HAL_OK) {
        Error_Handler("T: Failed to init TIM");
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK) {
        Error_Handler("T: Failed to init TIM");
    }
    if (HAL_TIM_PWM_Init(&htim1) != HAL_OK) {
        Error_Handler("T: Failed to init TIM");
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK) {
        Error_Handler("T: Failed to init TIM");
    }

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = period;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) {
        Error_Handler("T: Failed to init TIM");
    }
    sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
    sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
    sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
    sBreakDeadTimeConfig.DeadTime = 0;
    sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
    sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
    sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
    if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK) {
        Error_Handler("T: Failed to init TIM");
    }
}

static void ADC_init(void) {
    ADC_ChannelConfTypeDef sConfig = { 0 };

    hadc1.Instance = ADC_Instance;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T1_CC1;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;
    if (HAL_ADC_Init(&hadc1) != HAL_OK) {
        Error_Handler("T: Failed to init ADC");
    }

    /** Configure Regular Channel
     */
    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        Error_Handler("T: Failed to init ADC");
    }
}
