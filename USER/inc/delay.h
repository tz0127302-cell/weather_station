/**
 * @file delay.h
 * @brief Software delay function header file
 * @author Your Name
 * @date 2026-04-25
 */

#ifndef __DELAY_H
#define __DELAY_H

#include "main.h"

/* Function declarations */
void delay_us(u32 nus);  // 微秒级延时
void delay_ms(u32 nms);  // 毫秒级延时

#endif /* __DELAY_H */