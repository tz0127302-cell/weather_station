/**
 * @file key.h
 * @brief 按键检测函数头文件
 * @author He
 * @date 2026-04-25
 *
 * 本文件定义了按键扫描相关的宏、外部变量和函数声明。
 * 按键硬件连接: KEY1接在GPIOA的Pin 0引脚，低电平表示按键按下。
 */

#ifndef __KEY_H
#define __KEY_H

#include "main.h"

/* 宏定义 */


#define KEY1  (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0))

/* 外部变量声明 */


/* 函数声明 */

void Key_Init(void);


uint8_t Key_Scan(void);


#endif /* __KEY_H */
