#ifndef __DHT11_H
#define __DHT11_H

/**
 * @file dht11.h
 * @brief DHT11温湿度传感器驱动头文件
 * @details 定义DHT11单总线通信的IO控制宏和读写函数声明
 * @author He
 * @date 2026-04-25
 */

#include "main.h"

/* DHT11数据线控制宏 (连接至GPIOC的Pin1引脚) */
#define DHT11_DQ_OUT_H (GPIO_SetBits(GPIOC, GPIO_Pin_1))        /* 数据线输出高电平：将PC1置1 */
#define DHT11_DQ_OUT_L (GPIO_ResetBits(GPIOC, GPIO_Pin_1))     /* 数据线输出低电平：将PC1置0 */
#define DHT11_DQ_IN    (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1)) /* 读取数据线电平：返回PC1引脚的电平状态（0或1） */

/* 函数声明 */
uint8_t DHT11_Init(void);                           /* DHT11初始化函数：配置GPIO并完成传感器上电握手，返回0表示成功，非0表示失败 */
uint8_t DHT11_Read_Data(uint8_t *temp, uint8_t *humi); /* 读取温湿度数据：通过单总线协议读取40位数据，温度存入temp，湿度存入humi，返回0表示成功，非0表示失败 */

#endif
