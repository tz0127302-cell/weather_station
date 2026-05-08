#ifndef _MAIN_H
#define _MAIN_H

/**
 * @file main.h
 * @brief 主项目头文件
 * @details 包含所有硬件模块的头文件，统一管理项目引用
 * @author He
 * @date 2026-04-25
 */


/* 标准库和HAL头文件 */
#include "stm32f10x.h"       /* STM32F10x设备支持库 */
#include "stdio.h"           /* 标准输入输出库 */
#include "string.h"          /* 字符串处理库 */

/* 硬件驱动模块头文件 */
#include "led.h"             /* LED指示灯驱动 */
#include "usart1.h"          /* USART1串口驱动 */
#include "delay.h"           /* 延时函数 */
#include "lcd.h"             /* LCD显示驱动 */
#include "key.h"             /* 按键驱动 */
#include "spi.h"             /* SPI总线驱动 */
#include "dht11.h"           /* DHT11温度湿度传感器驱动 */
#include "w25qxx.h"          /* W25Q64存储芯片驱动 */
#include "su03t.h"           /* SU03T语音识别模块驱动 */
#include "my1680.h"          /* MY1680语音合成模块驱动 */
#include "rtc.h"             /* RTC实时时钟驱动 */
#include "wifi.h"            /* WIFI模块驱动 */
#include "weather.h"         /* 天气数据解析模块 */
#include "weather_voice.h"   /* 天气语音合成模块 */
#include "FreeRTOS.h"         /* FreeRTOS实时操作系统 */
#include "timers.h"          /* FreeRTOS定时器管理头文件 */
#include "task.h"            /* FreeRTOS任务管理头文件 */
#include "queue.h"           /* FreeRTOS队列管理头文件 */  
#include "timers.h"          /* FreeRTOS定时器管理头文件 */
#include "app.h"             /* 应用模块头文件 */

#endif /* _MAIN_H */
