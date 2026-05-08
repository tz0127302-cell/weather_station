/**
 * @file spi.c
 * @brief SPI控制器驱动源文件
 * @details SPI1的管脚初始化和数据交换
 * @author He
 * @date 2026-04-25
 */

#include "spi.h"

/**
  * @brief SPI1控制器初始化函数
  * @param  void
  * @retval void
  * @details GPIO管脚配置：
  *         PA5(SD_SCK) - 复用推挽输出
  *         PA6(SD_MISO) - 浮空输入
  *         PA7(SD_MOSI) - 复用推挽输出
  *         配置SPI为主模式、全双工、8bit数据
  * @author He
  */

void Spi1_Config(void)
{
    /* 步骤1：打开GPIOA时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    
    /* 步骤2：初始化GPIO结构体 */
    GPIO_InitTypeDef GPIO_InitStruct={0};
    
    /* 配置PA5(CLK)和PA7(MOSI)为复用推挽输出 */
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Speed= GPIO_Speed_50MHz;
    GPIO_Init(GPIOA,&GPIO_InitStruct);

    /* 配置PA6(MISO)为浮空输入 */
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
    GPIO_Init(GPIOA,&GPIO_InitStruct);
    
    /* 步骤3：打开SPI1时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);
   
    /* 步骤4：初始化SPI1参数 */
    SPI_InitTypeDef SPI_InitStruct={0};
    SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;  /* 分频系数为4，波特率=72M/4/2=9M */
    SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;                         /* 第一个时钟沿采样数据 */
    SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;                           /* 时钟空闲时为低电平 */
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;                    /* 8bit数据 */
    SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  /* 全双工，两条线 */
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;                   /* 高位先出 */
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;                        /* 主模式 */
    SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;                            /* 软件管理片选 */
    SPI_Init(SPI1,&SPI_InitStruct);
   
    /* 步骤5：使能SPI1 */
    SPI_Cmd(SPI1,ENABLE);
}


/**
  * @brief SPI1数据交换函数
  * @param  u8 data - 要发送的8bit数据
  * @retval u8 - 返回接收到的8bit数据
  * @details 全双工交换数据：发送一个字节的同时接收一个字节
  *         轮询等待发送标志和接收标志
  * @author He
  */

u8 Spi1_TransferData(u8 data)
{
    /* 等待发送缓冲区为空 */
    while(!SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE));  /* 轮询等待发送标志位为1 */
    
    /* 发送一个字节的数据 */
    SPI_I2S_SendData(SPI1,data);
    
    /* 等待接收缓冲区非空 */
    while(!SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE)); /* 轮询等待接收标志位为1 */
    
    /* 接收并返回数据 */
    return SPI_I2S_ReceiveData(SPI1);
}






