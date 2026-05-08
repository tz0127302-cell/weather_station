#ifndef __LED_H__
#define __LED_H__   
/**
 * @file led.h
 * @brief LED指示灯驱动头文件
 * @details 包含LED的GPIO控制宏定义和初始化函数声明
 * @author He
 * @date 2026-04-25
 */

#include "main.h"

/* LED控制宏定义 */
/* LED1控制 (PC5) */
#define LED1_ON  GPIO_ResetBits(GPIOC, GPIO_Pin_5);      /* LED1亮 */
#define LED1_OFF GPIO_SetBits(GPIOC, GPIO_Pin_5);       /* LED1灭 */

/* LED2控制 (PC4) */
#define LED2_ON  GPIO_ResetBits(GPIOC, GPIO_Pin_4);      /* LED2亮 */
#define LED2_OFF GPIO_SetBits(GPIOC, GPIO_Pin_4);       /* LED2灭 */

/* LED状态翻转宏定义 */
#define LED1_TOGGLE GPIOC->ODR ^= (1 << 5);             /* LED1反转 */
#define LED2_TOGGLE GPIOC->ODR ^= (1 << 4);             /* LED2反转 */

/* 函数声明 */
void Led_Init(void);                                    /* LED管脚初始化函数 */

#endif