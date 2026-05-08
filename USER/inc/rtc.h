#ifndef __RTC_H__
#define __RTC_H__

/**
 * @file rtc.h
 * @brief RTC实时时钟驱动头文件
 * @details 包含RTC初始化、时间设置、时间数据解析函数声明
 * @author He
 * @date 2026-04-25
 */

#include "main.h"

/* 函数声明 */
void Rtc_Init(void);                                    /* RTC初始化 */
void RTC_SetTimeandDate(u8 hour,u8 minute,u8 second,u8 day,u8 month,u16 year);  /* 设置时间日期 */
void RTC_Analysis(void);                                /* 时间日期数据解析 */

#endif
