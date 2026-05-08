/**
 * @file usart1.h
 * @brief USART1驱动头文件
 * @details USART1串口通信接口和调试输出功能
 * @author Your Name
 * @date 2026-04-25
 */

#ifndef __USART1_H
#define __USART1_H

#include "main.h"

/* USART1接收缓冲结构体 */
typedef struct
{
    u8 buff[256];       /* 接收缓冲区 */
    u16 len;            /* 接收数据长度 */
    u8 flag;            /* 接收标志位 */
}U1;

/* 函数声明 */
void USART1_Init(uint32_t baudrate);                /* USART1初始化 */
void USART1_SendByte(uint8_t data);                 /* 发送单个字节 */
void USART1_SendString(char *str);                  /* 发送字符串 */
void USART1_SendBuffer(uint8_t *buf, uint16_t len);  /* 发送数据缓冲区 */

#endif /* __USART1_H */