/**
 * @file delay.h
 * @brief 软件延时函数头文件
 * @author Your Name
 * @date 2026-04-25
 *
 * 本文件声明了基于软件循环的微秒级和毫秒级延时函数。
 * 延时精度依赖于系统时钟频率（通常在main.h中通过SYSCLK_FREQ定义）。
 */

#ifndef __DELAY_H
#define __DELAY_H

#include "main.h"

/* 函数声明 */

/*
 * delay_us - 微秒级延时函数
 * 功能: 通过软件循环实现指定微秒数的延时
 * 参数: nus - 需要延时的微秒数（最大值受定时器/计数器位数及系统时钟限制）
 * 返回值: 无
 * 注意: 实际延时精度取决于系统时钟频率和编译器优化等级
 */
void delay_us(u32 nus);

/*
 * delay_ms - 毫秒级延时函数
 * 功能: 通过软件循环或调用delay_us实现指定毫秒数的延时
 * 参数: nms - 需要延时的毫秒数
 * 返回值: 无
 * 注意: 该函数通常通过循环调用delay_us(1000)来实现毫秒级延时
 */
void delay_ms(u32 nms);

#endif /* __DELAY_H */
