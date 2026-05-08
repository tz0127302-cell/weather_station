/**
 * @file key.h
 * @brief 按键检测函数头文件
 * @author He
 * @date 2026-04-25
 */

#ifndef __KEY_H
#define __KEY_H

#include "main.h"

/* 宏定义 */
#define KEY1  (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0))

/* 外部变量 */
extern volatile uint8_t key_int_flag;

/* 函数声明 */
void Key_Init(void);
uint8_t Key_Scan(void);
void Key_EXTI_Init(void);

#endif /* __KEY_H */
