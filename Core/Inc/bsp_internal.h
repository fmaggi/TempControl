#ifndef __INTERNAL_H
#define __INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

void Error_Handler(const char* msg);

#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC

#define T_Pin GPIO_PIN_0
#define T_GPIO_Port GPIOA

#define LCD_LED_Pin GPIO_PIN_0
#define LCD_LED_GPIO_Port GPIOB
#define LCD_DC_Pin GPIO_PIN_1
#define LCD_DC_GPIO_Port GPIOB
#define LCD_RST_Pin GPIO_PIN_10
#define LCD_RST_GPIO_Port GPIOB
#define LCD_CS_Pin GPIO_PIN_11
#define LCD_CS_GPIO_Port GPIOB

#define Triac1_Pin GPIO_PIN_12
#define Triac1_GPIO_Port GPIOB
#define Triac2_Pin GPIO_PIN_13
#define Triac2_GPIO_Port GPIOB

#define ZC_Pin GPIO_PIN_14
#define ZC_GPIO_Port GPIOB
#define ZC_EXTI_IRQn EXTI15_10_IRQn

#define BTN_CLK_Pin GPIO_PIN_6
#define BTN_CLK_GPIO_Port GPIOB
#define BTN_DT_Pin GPIO_PIN_7
#define BTN_DT_GPIO_Port GPIOB

#define Ok_Pin GPIO_PIN_8
#define Ok_GPIO_Port GPIOB
#define Ok_EXTI_IRQn EXTI9_5_IRQn

#define TX_Pin GPIO_PIN_9
#define TX_GPIO_Port GPIOA
#define RX_Pin GPIO_PIN_10
#define RX_GPIO_Port GPIOA

#endif /* __INTERNAL_H */
