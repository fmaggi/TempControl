/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

#include "adc.h"
#include "dma.h"
#include "gpio.h"
#include "spi.h"
#include "tim.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ILI9341_GFX.h"
#include "ILI9341_STM32_Driver.h"
#include "stm32f1xx_hal_adc.h"

#include <stdint.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
// TIM3 TICK is 250 ps.
// Electrical network half cycle is 10 ms or 10000000 ps.
// At this TICK we can measure up to 16.38 ms. This is more than enough
// This also means that anything above 10 ms will never be reached, because
// INT due to zero corssing resets the clock
#define TICK               250
#define HALF_CYCLE_TIME    10000000 // ps. Duration of a half cycle of the power network at 50Hz
#define MAX_POWER          4096     // This is due to ADC conversion going from 0 to 4096
#define UNREACHABLE_PERIOD 0xFFFF
static const uint32_t MAX_PERIOD = HALF_CYCLE_TIME / TICK;
static const uint32_t POWER_TO_PERIOD = MAX_PERIOD / MAX_POWER;
static const uint32_t HALF_POWER = MAX_POWER / 2;

static volatile uint32_t power = 1800;

enum State {
    Entry1,
    Entry2,
    Entry3,
};

static volatile enum State current_state = Entry1;
static volatile enum State next_state = Entry1;
static volatile uint8_t ok_clicked = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

static uint32_t pid(uint32_t temp);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static volatile uint32_t temp = 0;

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_TIM3_Init();
    MX_SPI1_Init();
    MX_TIM4_Init();
    MX_TIM1_Init();
    MX_ADC1_Init();
    /* USER CODE BEGIN 2 */

    HAL_TIM_OC_Start_IT(&htim1, TIM_CHANNEL_1);
    HAL_ADC_Start_IT(&hadc1);

    __HAL_DBGMCU_FREEZE_TIM1();

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
        uint16_t t = TIM1->CNT;
        (void) t;
        HAL_Delay(1);
    }
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
    RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
    RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
        Error_Handler();
    }
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == ZC_Pin) {
        __disable_irq();
        Handle_Zero_Crossing();
        __enable_irq();
    } else if (GPIO_Pin == Ok_Pin) {
        ok_clicked = 1;
    } else {
        __NOP();
    }
}

void Handle_Zero_Crossing(void) {
    TIM3->CNT = 0x0;
    TIM3->CCR1 = UNREACHABLE_PERIOD; // this should never be reached
    TIM3->CCR2 = UNREACHABLE_PERIOD; // this shoudl never be reached

    {
        uint32_t power1 = power > HALF_POWER ? MAX_POWER : power * 2;
        TIM3->CCR1 = (MAX_POWER - power1) * POWER_TO_PERIOD;
    }

    {
        uint32_t power2 = power >= HALF_POWER ? (power - HALF_POWER) * 2 : 0;
        TIM3->CCR2 = (MAX_POWER - power2) * POWER_TO_PERIOD;
    }
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
        htim->Instance->CCR1 = UNREACHABLE_PERIOD;
        Trigger_Triac(Triac1_GPIO_Port, Triac1_Pin);
    } else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
        htim->Instance->CCR2 = UNREACHABLE_PERIOD;
        Trigger_Triac(Triac2_GPIO_Port, Triac2_Pin);
    } else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3) {
        // If we get here it's an error
        htim->Instance->CNT = 0x0;
        HAL_TIM_OC_Stop_IT(&htim3, TIM_CHANNEL_1);
        HAL_TIM_OC_Stop_IT(&htim3, TIM_CHANNEL_2);
        HAL_TIM_OC_Stop_IT(&htim3, TIM_CHANNEL_3);

        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
        for (;;) {}
    } else {
        __NOP();
    }
    __enable_irq();
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    temp = HAL_ADC_GetValue(hadc);
}

static uint32_t pid(uint32_t t) {
    return t;
    if (power >= MAX_POWER) {
        return 205;
    }
    return power + 1;
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {}
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line) {
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line
       number, printf("Wrong parameters value: file %s on line %d\r\n", file,
       line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
