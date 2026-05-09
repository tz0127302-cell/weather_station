#ifndef _W25Qxx_H
#define _W25Qxx_H

/**
 * @file w25qxx.h
 * @brief W25Q64 Flash存储器驱动头文件
 * @details 定义W25Q64（8MB SPI Flash）的片选控制宏和读写擦除函数声明
 * @author He
 * @date 2026-04-25
 */

/*******************文件包含***********************/

#include "main.h"

/*******************宏定义***********************/

/* W25Q64片选控制 (CS - 连接至GPIOA的Pin4引脚) */
#define W25Q64_CS_H   (GPIO_SetBits(GPIOA,GPIO_Pin_4))   /* 片选拉高：取消选择W25Q64 */
#define W25Q64_CS_L   (GPIO_ResetBits(GPIOA,GPIO_Pin_4)) /* 片选拉低：选中W25Q64，允许SPI通信 */

/*******************外部变量声明***********************/
extern u8 file_flag;        /* 文件标志位：指示字库文件是否存在/已更新，0表示未就绪，1表示就绪 */
extern u8 transfer_flag;    /* 传输标志位：指示SPI数据传输状态，0表示空闲，1表示正在传输 */

/*******************函数声明***********************/
void W25Qxx_Config(void);                           /* W25Q64初始化配置函数：配置SPI引脚、模式和参数 */
void W25q64_ReadID(void);                           /* 读取芯片ID函数：读取W25Q64的制造商ID和设备ID，用于验证通信是否正常 */
void Sector_Erase(u32 addr);                        /* 扇区擦除函数：擦除指定地址所在的扇区（每个扇区4KB） */
void HalfBlock_Erase(u32 addr);                     /* 半块擦除函数：擦除指定地址所在的半块（32KB） */
void Block_Erase(u32 addr);                         /* 块擦除函数：擦除指定地址所在的块（64KB） */
void Chip_Erase(void);                              /* 全片擦除函数：擦除整个Flash芯片的全部存储空间 */
void Page_Write(u32 addr,u32 len,u8 *data);         /* 页写入函数：向指定地址写入一页数据（一页256字节），最多写入一页 */
void Read_Data(u32 addr,u32 len,u8 *data);          /* 数据读取函数：从指定地址读取指定长度的数据到缓冲区 */
void W25Q64_CrossPageWrite(u32 addr,u32 len,u8 *data); /* 跨页写入函数：自动处理跨页边界的多页连续写入，写入长度不受一页限制 */
void Font_Update(void);                             /* 字库更新函数：从外部介质更新Flash中存储的字库数据 */

#endif
