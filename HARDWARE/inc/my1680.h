#ifndef __MY1680_H__
#define __MY1680_H__
/**
 * @file my1680.h
 * @brief MY1680语音模块驱动头文件
 * @details 通过UART串口控制MY1680语音模块播放/停止语音
 * @author He
 * @date 2026-04-25
 */

#include "main.h"

/* MY1680忙状态检测引脚 (连接至GPIOB的Pin8引脚) */
#define MY1680_BUSY (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8))  /* 读取BUSY引脚电平：低电平表示模块正在播放语音，高电平表示空闲 */

/* 语音指令数组 */
extern u8 play[5];      /* 播放指令数组：通过UART发送给MY1680的播放命令字节序列 */
extern u8 stop[5];      /* 停止指令数组：通过UART发送给MY1680的停止播放命令字节序列 */

/* 函数声明 */
void MY1680_Config(void);                               /* MY1680初始化配置函数：配置与MY1680通信的UART引脚和参数 */
void UART5_SendByte(uint8_t data);                      /* UART5发送单字节函数：通过UART5串口发送一个字节数据 */
void UART5_SendBuff(uint8_t *buff, uint8_t len);        /* UART5发送缓冲区函数：通过UART5串口发送指定长度的字节数据 */
void MY1680_Play(u8 dir_name, u8 file_name);            /* 播放指定语音文件函数：dir_name为目录编号，file_name为文件编号，组合成指令发送给MY1680 */

#endif
