#include "bsp.h"

#include "adc.h"
#include "dma.h"
#include "gpio.h"
#include "spi.h"
#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_adc.h"
#include "stm32f1xx_hal_adc_ex.h"
#include "stm32f1xx_hal_cortex.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_tim.h"
#include "tim.h"

#include <bsp_internal.h>
#include <stdint.h>
#include <stdio.h>

static volatile uint8_t ok_clicked = 0;

void SystemClock_Config(void);


inline void BSP_init(void) {
    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    MX_DMA_Init();

    // Triacs
    MX_TIM3_Init();

    MX_SPI1_Init();

    // Encoder
    MX_TIM4_Init();

    // TIM1 has a 0.1 s period. At this rate we sample ADC
    MX_TIM1_Init();
    MX_ADC1_Init();

    __HAL_DBGMCU_FREEZE_TIM1();
    __HAL_DBGMCU_FREEZE_TIM3();
    __HAL_DBGMCU_FREEZE_TIM4();

    BSP_Display_init();

    HAL_TIM_Encoder_Start_IT(&htim4, TIM_CHANNEL_ALL);

    HAL_ADCEx_Calibration_Start(&hadc1);
    // Calibration enables adc
    HAL_ADC_Stop(&hadc1);
}

void BSP_delay(uint32_t ms) {
    HAL_Delay(ms);
}

uint32_t BSP_millis(void) {
    return HAL_GetTick();
}

uint8_t BSP_get_cursor(uint8_t current_cursor) {
    (void) current_cursor;
    return (uint8_t) TIM4->CNT;
}

uint8_t BSP_ok_clicked(void) {
    uint8_t temp = ok_clicked;
    ok_clicked = 0;
    return temp;
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

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    switch (GPIO_Pin) {
        case ZC_Pin: {
            BSP_Power_ZC_interrupt();
            break;
        }
        case Ok_Pin: {
            ok_clicked = 1;
            break;
        }
        default: break;
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
        Error("404: 220V not found!");
    } else {
        __NOP();
    }
    __enable_irq();
}


__weak void Error(const char* msg) {
    BSP_Display_clear(RED);
    BSP_Display_write_text(msg, 0, 0, FONT3, WHITE, RED);
    __disable_irq();
    while (1) {}
}

void Error_Handler() {
    Error("System error");
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
