#ifndef __RTC_H__
#define __RTC_H__

/**
 * @file rtc.h
 * @brief RTC实时时钟功能头文件
 * @details 包含RTC初始化、时间设置、时间数据解析等功能
 * @author He
 * @date 2026-04-25
 */

#include "main.h"

/*
 * RTC_TimeTypeDef - RTC实时时钟数据结构体
 * 功能: 用于存储RTC模块解析后的完整日期和时间信息
 * 说明: 该结构体通常由RTC_Analysis函数填充，
 *        供上层应用（如显示任务、数据记录等）使用
 *
 * 字段说明:
 *   year   - 年份（范围: 1970~2106，取决于RTC硬件支持的年份范围）
 *   month  - 月份（范围: 1~12）
 *   day    - 日期（范围: 1~31，取决于具体月份）
 *   hour   - 小时（范围: 0~23，24小时制）
 *   minute - 分钟（范围: 0~59）
 *   second - 秒钟（范围: 0~59）
 */
typedef struct
{
    u16 year;   /* 年份 */
    u8  month;  /* 月份 */
    u8  day;    /* 日期 */
    u8  hour;   /* 小时（24小时制） */
    u8  minute; /* 分钟 */
    u8  second; /* 秒钟 */
} RTC_TimeTypeDef;

/*
 * rtc_time - RTC时间数据结构体变量（外部引用）
 * 功能: 全局RTC时间变量，在rtc.c中定义，
 *       其他模块通过 extern 声明访问当前RTC时间
 * 说明: 该变量由RTC_Analysis函数周期性更新，
 *       显示任务等模块直接读取此变量获取当前时间
 */
extern RTC_TimeTypeDef rtc_time;

/* 函数声明 */

/*
 * Rtc_Init - RTC初始化函数
 * 功能: 初始化STM32的RTC外设，配置时钟源（通常使用LSE外部低速晶振），
 *       使能RTC时钟，并等待RTC寄存器同步
 * 参数: 无
 * 返回值: 无
 * 说明: 该函数需要在系统启动时调用一次；
 *       如果RTC已经初始化过（VBAT保持供电），则跳过重新初始化
 */
void Rtc_Init(void);

/*
 * RTC_SetTimeandDate - 设置RTC时间和日期
 * 功能: 将指定的年、月、日、时、分、秒写入RTC寄存器，
 *       更新RTC模块的当前时间
 * 参数:
 *   hour   - 小时（0~23）
 *   minute - 分钟（0~59）
 *   second - 秒钟（0~59）
 *   day    - 日期（1~31）
 *   month  - 月份（1~12）
 *   year   - 年份
 * 返回值: 无
 */
void RTC_SetTimeandDate(u8 hour, u8 minute, u8 second, u8 day, u8 month, u16 year);

/*
 * RTC_Analysis - RTC时间数据解析函数
 * 功能: 从RTC硬件寄存器中读取当前的秒、分、时、日、月、年计数，
 *       将其转换为RTC_TimeTypeDef结构体格式，并存储到全局变量rtc_time中
 * 参数: 无
 * 返回值: 无
 * 说明: 该函数需要周期性调用（通常在定时器中断或任务循环中），
 *       以保证rtc_time变量中的时间为最新值
 */
void RTC_Analysis(void);

#endif
