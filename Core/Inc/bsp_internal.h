#ifndef __INTERNAL_H
#define __INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

void Error_Handler(void);

#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define T_Pin GPIO_PIN_0
#define T_GPIO_Port GPIOA
#define ZC_Pin GPIO_PIN_1
#define ZC_GPIO_Port GPIOA
#define ZC_EXTI_IRQn EXTI1_IRQn
#define Triac1_Pin GPIO_PIN_3
#define Triac1_GPIO_Port GPIOA
#define Triac2_Pin GPIO_PIN_4
#define Triac2_GPIO_Port GPIOA
#define LCD_LED_Pin GPIO_PIN_0
#define LCD_LED_GPIO_Port GPIOB
#define LCD_RST_Pin GPIO_PIN_1
#define LCD_RST_GPIO_Port GPIOB
#define LCD_DC_Pin GPIO_PIN_10
#define LCD_DC_GPIO_Port GPIOB
#define LCD_CS_Pin GPIO_PIN_11
#define LCD_CS_GPIO_Port GPIOB
#define Ok_Pin GPIO_PIN_14
#define Ok_GPIO_Port GPIOB
#define Ok_EXTI_IRQn EXTI15_10_IRQn
#define BTN_CLK_Pin GPIO_PIN_6
#define BTN_CLK_GPIO_Port GPIOB
#define BTN_DT_Pin GPIO_PIN_7
#define BTN_DT_GPIO_Port GPIOB

#endif /* __INTERNAL_H */
