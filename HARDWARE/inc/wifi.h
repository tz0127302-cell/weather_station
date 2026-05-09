#ifndef _WIFI_H
#define _WIFI_H

/**
 * @file wifi.h
 * @brief WiFi模块驱动头文件
 * @details 定义WiFi模块的UART接口函数和NTP时间同步、天气获取功能
 * @author He
 * @date 2026-04-25
 */

/*******************文件包含***********************/
#include "main.h"

/*******************结构体定义***********************/
/* USART3串口接收数据结构体 */
typedef struct
{
    u8 buff[1024];      /* 接收缓冲区：存储从WiFi模块（USART3）收到的原始数据 */
    u16 len;            /* 当前写入位置：指示缓冲区中下一个空闲位置的索引 */
    u16 frame_len;      /* 一帧数据结束时的数据长度：记录完整一帧数据的字节数 */
    u8 flag;            /* 接收标志位：0表示未收到完整数据帧，1表示已收到完整数据帧待处理 */
}U3;

/*******************宏定义***********************/
/* 天气API调用参数 */
#define WEATHER_KEY     "9akdmpg9548d1mqe"      /* 心知天气API密钥：用于身份认证和API调用计费 */

/*******************外部变量声明***********************/
extern U3 u3;           /* USART3串口接收数据结构体实例：存储WiFi模块返回的数据 */
extern u32 timestamp;   /* NTP时间戳：从NTP服务器同步获取的Unix时间戳（秒数） */
extern u8 ntp_flag;     /* NTP同步标志位：0表示未完成同步，1表示NTP时间同步成功 */

/*******************函数声明***********************/
void Usart3_Config(void);                   /* USART3初始化配置函数：配置与WiFi模块通信的串口引脚、波特率等参数 */
void usart3_send_byte(u8 data);             /* USART3发送单字节函数：通过USART3发送一个字节数据给WiFi模块 */
void usart3_send_str(char *str);            /* USART3发送字符串函数：通过USART3发送以'\0'结尾的字符串给WiFi模块 */
u8 Wifi_NtpInit(void);                      /* WiFi模块NTP初始化函数：配置ESP8266连接WiFi并同步NTP时间，返回0表示成功，非0表示失败 */
void Wifi_Control(void);                    /* WiFi控制主函数：处理WiFi模块的状态机，包括连接管理、数据收发等 */
void TH_CheckandPublish(void);              /* 温湿度采集与发布函数：读取DHT11数据并通过WiFi上报到云端 */
u8 Wifi_GetWeather(const char *city_code, char *response, u16 max_len);  /* 获取天气数据函数：根据城市代码请求天气API，结果存入response缓冲区，max_len为缓冲区最大长度，返回0表示成功，非0表示失败 */

#endif
