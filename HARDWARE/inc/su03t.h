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
    u8 buff[4];         /* 接收缓冲区，存储从SU03T模块收到的原始字节数据 */
    u16 len;            /* 数据长度，当前已接收的有效数据字节数 */
    u8 flag;            /* 接收标志位，0表示未收到完整数据帧，1表示已收到完整数据帧 */
}SU03T_Data;

extern volatile SU03T_Data su03t;                       /* SU03T接收数据结构体，ISR与任务共享，需volatile */

/* 语音播报状态标志: 防止播报声音被SU-03T误识别为指令
 * volatile: WeatherTask写入, KeyTask读取, 跨任务共享必须volatile防止寄存器缓存 */
extern volatile u8 voice_busy;                          /* 播报忙标志: 1=正在播报 0=空闲, 忙时只响应暂停/停止 */
extern volatile u8 voice_abort;                         /* 播报中止标志: 1=需要中止当前播报, 由暂停/停止指令设置 */

/* 全局函数 */
void SU_03T_Config(void);
void SU_03T_Control(void);

/* 播报期间禁用/恢复SU-03T UART4中断: 从硬件层面防止播报音频被误识别 */
void SU03T_DisableIRQ(void);    /* 禁用UART4中断，播报期间不接收任何语音指令 */
void SU03T_EnableIRQ(void);     /* 恢复UART4中断并清空残留数据 */

#endif
