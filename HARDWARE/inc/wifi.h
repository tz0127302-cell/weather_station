#ifndef _WIFI_H
#define _WIFI_H

/**
 * @file wifi.h
 * @brief WIFI模块驱动头文件
 * @details 包含WIFI模块的UART接口函数和NTP时间同步
 * @author He
 * @date 2026-04-25
 */

/*******************文件包含***********************/
#include "main.h"

/*******************结构体定义***********************/
/* USART3接收数据结构体 */
typedef struct
{
    u8 buff[1024];      /* 接收缓冲区 */
    u16 len;            /* 当前写入位置 */
    u16 frame_len;      /* 一帧数据接收完成时数据长度 */
    u8 flag;            /* 接收标志位 (0:未完成, 1:已完成) */
}U3;

/*******************宏定义***********************/
/* 天气API配置参数 */
#define WEATHER_KEY     "9akdmpg9548d1mqe"      /* 天气API key */

/*******************外部变量声明***********************/
extern U3 u3;           /* USART3接收数据结构体 */
extern u32 timestamp;   /* NTP时间戳 */
extern u8 ntp_flag;     /* NTP同步标志位 */

/*******************函数声明***********************/
void Usart3_Config(void);                   /* USART3配置初始化 */
void usart3_send_byte(u8 data);             /* USART3发送单字节 */
void usart3_send_str(char *str);            /* USART3发送字符串 */
u8 Wifi_NtpInit(void);                      /* WIFI模块NTP初始化 */
void Wifi_Control(void);                    /* WIFI控制函数 */
void TH_CheckandPublish(void);              /* 温湿度检测与发布 */
u8 Wifi_GetWeather(const char *city_code, char *response, u16 max_len);  /* 获取天气数据 */

#endif
