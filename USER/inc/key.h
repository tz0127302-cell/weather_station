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

/*
 * KEY1 - 读取按键KEY1的输入电平状态
 * 功能: 通过读取GPIOA引脚0（PA0）的输入数据位来获取按键状态
 * 返回值: Bit_RESET（0）= 按键按下（低电平），Bit_SET（1）= 按键未按下（高电平）
 * 说明: 该宏直接映射到硬件寄存器，实时反映引脚电平
 */
#define KEY1  (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0))

/* 外部变量声明 */

/*
 * key_int_flag - 按键中断标志位
 * 功能: 在按键外部中断服务函数中被置位（通常设为1），
 *       主循环或按键扫描任务中检测该标志后执行按键处理，
 *       处理完成后需软件清零
 * 类型: volatile uint8_t（volatile防止编译器优化，确保每次从内存读取）
 */
extern volatile uint8_t key_int_flag;

/* 函数声明 */

/*
 * Key_Init - 按键GPIO初始化
 * 功能: 配置按键引脚的GPIO模式（通常为上拉输入或浮空输入），
 *       使能相关GPIO时钟
 * 参数: 无
 * 返回值: 无
 */
void Key_Init(void);

/*
 * Key_Scan - 按键扫描函数
 * 功能: 检测按键状态，支持消抖处理，返回按键键值
 * 参数: 无
 * 返回值: uint8_t - 按键键值（0表示无按键按下，非0表示对应按键按下）
 * 说明: 该函数通常在主循环或按键任务中周期调用
 */
uint8_t Key_Scan(void);

/*
 * Key_EXTI_Init - 按键外部中断初始化
 * 功能: 配置按键对应的外部中断线（EXTI），设置触发方式（通常为下降沿触发），
 *       配置NVIC中断优先级并使能中断
 * 参数: 无
 * 返回值: 无
 * 说明: 初始化后，按键按下将触发外部中断，中断服务函数中置位key_int_flag
 */
void Key_EXTI_Init(void);

#endif /* __KEY_H */
