#ifndef _W25Qxx_H
#define _W25Qxx_H

/**
 * @file w25qxx.h
 * @brief W25Q64模块驱动头文件
 * @details 包含W25Q64 FLASH存储器的控制函数定义
 * @author He
 * @date 2026-04-25
 */

/*******************文件包含***********************/
#include "main.h"

/*******************宏定义***********************/
/* W25Q64片选信号控制宏定义 (PA4) */
#define W25Q64_CS_H   (GPIO_SetBits(GPIOA,GPIO_Pin_4))    /* 片选信号拉高 */
#define W25Q64_CS_L   (GPIO_ResetBits(GPIOA,GPIO_Pin_4))  /* 片选信号拉低 */

/*******************外部变量声明***********************/
extern u8 file_flag;        /* 文件标志位(0:待发送, 1:发送中) */
extern u8 transfer_flag;    /* 传输标志(0:完成, 1:进行中) */

/*******************函数声明***********************/
void W25Qxx_Config(void);                           /* W25Q64配置初始化 */
void W25q64_ReadID(void);                           /* 读W25Q64的ID */
void Sector_Erase(u32 addr);                        /* 扇区擦除 (4KB) */
void HalfBlock_Erase(u32 addr);                     /* 半块擦除 (32KB) */
void Block_Erase(u32 addr);                         /* 块擦除 (64KB) */
void Chip_Erase(void);                              /* 全片擦除 */
void Page_Write(u32 addr,u32 len,u8 *data);         /* 页写 (256字节) */
void Read_Data(u32 addr,u32 len,u8 *data);          /* 读数据 */
void W25Q64_CrossPageWrite(u32 addr,u32 len,u8 *data);  /* 跨页写 */
void Font_Update(void);                             /* 字库更新 */

#endif
