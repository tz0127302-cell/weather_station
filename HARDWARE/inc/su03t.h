#ifndef __SU03T_H__
#define __SU03T_H__
/**
 * @file su03t.h
 * @brief SU03T语音识别模块驱动头文件
 * @details 接收并解析SU03T语音识别模块返回的UART数据
 * @author He
 * @date 2026-04-25
 */

#include "main.h"

/* SU03T语音识别数据接收结构体 */
typedef struct
{
    u8 buff[4];         /* 接收缓冲区：存储从SU03T模块收到的原始字节数据 */
    u16 len;            /* 数据长度：当前已接收到的数据字节数 */
    u8 flag;            /* 接收标志位：0表示未接收到完整数据帧，1表示已收到完整数据帧待处理 */
}SU03T_Data;

/* 函数声明 */
void SU_03T_Config(void);                               /* SU03T模块初始化配置函数：配置与SU03T通信的UART引脚和参数 */
void SU_03T_Control(void);                              /* 语音识别控制处理函数：解析SU03T返回的指令数据，根据识别结果执行相应操作 */

#endif
