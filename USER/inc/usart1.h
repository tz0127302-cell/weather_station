/**
 * @file usart1.h
 * @brief USART1串口通信头文件
 * @details USART1串口通信接口和接收缓冲区结构体定义
 * @author Your Name
 * @date 2026-04-25
 *
 * 本文件定义了USART1串口的接收缓冲区结构体和相关函数声明，
 * 用于串口数据收发（如与电脑上位机或串口模块通信）。
 */

#ifndef __USART1_H
#define __USART1_H

#include "main.h"

/*
 * U1 - USART1接收缓冲区结构体
 * 功能: 用于存储通过USART1串口接收到的数据，包括缓冲数组、
 *       数据长度和接收状态标志
 *
 * 字段说明:
 *   buff[256] - 接收数据环形缓冲区（256字节），
 *               由串口中断服务函数逐字节填充
 *   len       - 当前接收到的数据长度（字节数），
 *               在接收过程中递增，收到完整数据帧后锁定
 *   flag      - 接收完成标志位，
 *               0 = 正在接收/未完成，1 = 已接收完整数据帧
 *               主循环或任务检测到该标志为1时处理数据，
 *               处理完成后需软件清零
 */
typedef struct
{
    u8 buff[256];       /* 接收数据缓冲区（最大256字节） */
    u16 len;            /* 已接收数据的长度 */
    u8 flag;            /* 接收完成标志（0:未完成, 1:已完成） */
} U1;

/* 函数声明 */

/*
 * USART1_Init - USART1串口初始化
 * 功能: 配置USART1的工作参数，包括波特率、数据位（8位）、
 *       停止位（1位）、校验位（无）、流控制（无），
 *       并使能发送和接收功能
 * 参数: baudrate - 串口通信波特率（如9600、115200等）
 * 返回值: 无
 * 说明: 该函数同时配置USART1的GPIO引脚（TX为复用推挽输出，
 *       RX为浮空输入）和NVIC中断优先级
 */
void USART1_Init(uint32_t baudrate);

/*
 * USART1_SendByte - USART1发送单个字节
 * 功能: 通过USART1串口发送一个字节数据
 * 参数: data - 要发送的字节数据
 * 返回值: 无
 * 说明: 该函数会等待发送数据寄存器为空（TXE标志）后再写入数据，
 *       为阻塞发送模式
 */
void USART1_SendByte(uint8_t data);

/*
 * USART1_SendString - USART1发送字符串
 * 功能: 通过USART1串口逐字节发送一个以'\0'结尾的字符串
 * 参数: str - 指向要发送字符串的指针
 * 返回值: 无
 * 说明: 该函数循环调用USART1_SendByte发送每个字符，
 *       直到遇到字符串结束符'\0'为止
 */
void USART1_SendString(char *str);

/*
 * USART1_SendBuffer - USART1发送数据缓冲区
 * 功能: 通过USART1串口发送指定长度的数据缓冲区内容
 * 参数:
 *   buf - 指向待发送数据缓冲区的指针
 *   len - 待发送数据的长度（字节数）
 * 返回值: 无
 * 说明: 该函数适用于发送非文本数据（如二进制数据包），
 *       循环调用USART1_SendByte发送len个字节
 */
void USART1_SendBuffer(uint8_t *buf, uint16_t len);

#endif /* __USART1_H */
