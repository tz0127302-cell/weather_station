#ifndef _MAIN_H
#define _MAIN_H

/**
 * @file main.h
 * @brief 项目总头文件
 * @details 集中管理所有硬件模块和FreeRTOS的头文件引用
 *          所有源文件只需 #include "main.h" 即可使用全部模块
 * @author He
 * @date 2026-04-25
 */

/*========================== 标准库和HAL库 ===========================*/

#include "stm32f10x.h"       /* STM32F10x 标准外设库: GPIO, USART, SPI 等寄存器定义 */
#include "stdio.h"           /* 标准输入输出: printf, sprintf 等 */
#include "string.h"          /* 字符串处理: memset, strstr, memcpy 等 */

/*========================== 硬件驱动模块 ===========================*/

#include "led.h"             /* LED指示灯驱动: LED1(PC5), LED2(PC4), 提供 ON/OFF/TOGGLE 宏 */
#include "usart1.h"          /* USART1串口驱动: 控制台输出(printf重定向), 波特率115200 */
#include "delay.h"           /* 软件延时: delay_us(微秒), delay_ms(毫秒), NOP循环实现 */
#include "lcd.h"             /* LCD显示屏驱动: ST7789, 320x240, 16位色, 软件SPI驱动 */
#include "key.h"             /* 按键驱动: PA0, 提供 Key_Scan() 和 EXTI 中断初始化 */
#include "spi.h"             /* 硬件SPI驱动: 用于 W25Q64 Flash 通信 */
#include "dht11.h"           /* DHT11温湿度传感器驱动: PC1, 单总线协议, 提供温湿度读取 */
#include "w25qxx.h"          /* W25Q64 SPI Flash存储驱动: 存储字库/图片/语音文件 */
#include "su03t.h"           /* SU03T语音识别模块驱动: UART4, 9600bps, 接收语音命令帧 */
#include "my1680.h"          /* MY1680语音合成模块驱动: UART5, 9600bps, 播放指定音频文件 */
#include "rtc.h"             /* RTC实时时钟驱动: 内部RTC, 1Hz, 提供日期时间功能 */
#include "wifi.h"            /* WiFi模块驱动: ESP-12F, USART3, AT指令, 含NTP和天气API */
#include "weather.h"         /* 天气数据解析: 含 WeatherData 结构体和 JSON 解析函数 */
#include "weather_voice.h"   /* 天气语音播报: 将天气数据组合成 MY1680 语音播放序列 */

/*========================== FreeRTOS 系统 ===========================*/

#include "FreeRTOS.h"        /* FreeRTOS 主头文件: 数据类型, 配置宏等 */
#include "timers.h"          /* 软件定时器: xTimerCreate, xTimerStart 等 API */
#include "task.h"            /* 任务管理: xTaskCreate, vTaskDelay 等 API */
#include "queue.h"           /* 队列通信: xQueueCreate, xQueueSend, xQueueReceive 等 */
#include "semphr.h"          /* 信号量: 二值信号量/互斥信号量/计数信号量 API */

/*========================== 应用层头文件 ===========================*/

#include "app.h"             /* 应用任务层: 包含3个任务+1个定时器的定义和创建 */

#endif /* _MAIN_H */
