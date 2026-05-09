#ifndef __LED_H__
#define __LED_H__
/**
 * @file led.h
 * @brief LED指示灯驱动头文件
 * @details 定义LED相关的GPIO控制宏和初始化函数声明
 * @author He
 * @date 2026-04-25
 */

#include "main.h"

/* LED控制宏定义 */

/* LED1控制 (连接至GPIOC的Pin5引脚) */
#define LED1_ON  GPIO_ResetBits(GPIOC, GPIO_Pin_5);      /* LED1点亮：将PC5输出低电平（低电平有效） */
#define LED1_OFF GPIO_SetBits(GPIOC, GPIO_Pin_5);       /* LED1熄灭：将PC5输出高电平 */

/* LED2控制 (连接至GPIOC的Pin4引脚) */
#define LED2_ON  GPIO_ResetBits(GPIOC, GPIO_Pin_4);      /* LED2点亮：将PC4输出低电平（低电平有效） */
#define LED2_OFF GPIO_SetBits(GPIOC, GPIO_Pin_4);       /* LED2熄灭：将PC4输出高电平 */

/* LED状态翻转宏定义 */
#define LED1_TOGGLE GPIOC->ODR ^= (1 << 5);             /* LED1翻转：通过异或操作切换PC5输出电平 */
#define LED2_TOGGLE GPIOC->ODR ^= (1 << 4);             /* LED2翻转：通过异或操作切换PC4输出电平 */

/* 函数声明 */
void Led_Init(void);                                    /* LED引脚初始化函数：配置PC4、PC5为推挽输出模式 */

#endif
