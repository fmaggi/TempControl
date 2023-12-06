#include "bsp.h"

#include "bsp_internal.h"
#include "cmsis_gcc.h"
#include "stm32f1xx_hal_flash.h"
#include "stm32f1xx_hal_flash_ex.h"

#include <stdint.h>
#include <stdio.h>

extern uint32_t _user_data_start;
#define FLASH_START ((uint32_t)&_user_data_start)

void SystemClock_Config(void);

void BSP_init(void) {
    HAL_Init();

    SystemClock_Config();

    BSP_Display_init();
    BSP_Power_init();
    BSP_T_init(100); // sample every 100 ms
    BSP_IO_init();
}

void BSP_delay(uint32_t ms) {
    HAL_Delay(ms);
}

uint32_t BSP_millis(void) {
    return HAL_GetTick();
}

// NOTE: should do something with error
void BSP_Flash_write(void* address_start, uint32_t numberofwords, uint32_t* data) {
    static FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PAGEError = 0;

    HAL_FLASH_Unlock();

    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = FLASH_START;
    EraseInitStruct.NbPages = 1;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK) {
        goto ret;
    }

    for (uint32_t i = 0; i<numberofwords; ++i) {
        uint32_t address = (uint32_t)address_start + i*4;
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data[i]) != HAL_OK) {
            PAGEError = HAL_FLASH_GetError();
            goto ret;
        }

    }

ret:
    HAL_FLASH_Lock();
    return;
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
        Error_Handler("BSP: Failed to init HSI");
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
        Error_Handler("BSP: Failed to init clocks");
    }

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
        Error_Handler("BSP: Failed to init clocks");
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    switch (GPIO_Pin) {
        case ZC_Pin: {
            __disable_irq();
            BSP_Power_ZC_interrupt();
            __enable_irq();
            break;
        }
        case Ok_Pin: {
            BSP_IO_Ok_interrupt();
            break;
        }
        default: break;
    }
}

__weak void Error_Handler(const char* msg) {
    BSP_Display_clear(RED);
    BSP_Display_write_text(msg, 0, 0, FONT3, WHITE, RED);
    __disable_irq();
    while (1) {}
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
