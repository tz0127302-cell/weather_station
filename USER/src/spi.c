/**
 * @file spi.c
 * @brief SPI硬件驱动函数源文件
 * @details SPI1的引脚初始化与数据收发驱动，用于与W25Q64等SPI设备通信
 * @author He
 * @date 2026-04-25
 */

#include "spi.h"

/**
  * @brief SPI1硬件接口初始化配置函数
  * @param  void
  * @retval void
  * @details GPIO引脚配置如下：
  *         PA5(SCK)  - 串行时钟，复用推挽输出
  *         PA6(MISO) - 主入从出，浮空输入
  *         PA7(MOSI) - 主出从入，复用推挽输出
  *         SPI配置为主机模式、全双工、8位数据、MSB先传、
  *         时钟空闲低电平、第一个时钟沿采样、软件NSS管理
  * @note SPI时钟频率 = 72MHz / 4 = 18MHz，但SPI协议还需再/2，
  *       因此实际SCK频率 = 72M/4/2 = 9MHz
  * @author He
  */
void Spi1_Config(void)
{
    /* 步骤1：使能GPIOA外设时钟，PA5/PA6/PA7属于GPIOA */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);

    /* 步骤2：初始化GPIO引脚结构体 */
    GPIO_InitTypeDef GPIO_InitStruct={0};

    /* 配置PA5(SCK时钟线)和PA7(MOSI主出从入)为复用推挽输出模式 */
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;        /* 复用推挽输出，由片上外设控制引脚 */
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7; /* PA5(SCK) + PA7(MOSI) */
    GPIO_InitStruct.GPIO_Speed= GPIO_Speed_50MHz;       /* IO口翻转速度50MHz */
    GPIO_Init(GPIOA,&GPIO_InitStruct);

    /* 配置PA6(MISO主入从出)为浮空输入模式 */
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;  /* 浮空输入，由外部SPI从设备驱动 */
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;              /* PA6(MISO) */
    GPIO_Init(GPIOA,&GPIO_InitStruct);

    /* 步骤3：使能SPI1外设时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);

    /* 步骤4：配置SPI1通信参数 */
    SPI_InitTypeDef SPI_InitStruct={0};
    SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;  /* 波特率预分频=4，SPI时钟=72M/4/2=9MHz */
    SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;                         /* 时钟相位：第一个时钟沿采样数据 */
    SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;                           /* 时钟极性：空闲时SCK为低电平 */
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;                    /* 数据帧长度：8位 */
    SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;   /* 双线全双工模式（同时收发） */
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;                   /* 数据传输顺序：高位（MSB）先传输 */
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;                        /* SPI工作模式：主机模式 */
    SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;                            /* NSS片选信号：软件管理（由GPIO控制） */
    SPI_Init(SPI1,&SPI_InitStruct);

    /* 步骤5：使能SPI1外设，开始工作 */
    SPI_Cmd(SPI1,ENABLE);
}


/**
  * @brief SPI1数据收发函数（全双工）
  * @param  u8 data - 要发送的8位数据字节
  * @retval u8 - 返回从从设备接收到的8位数据
  * @details SPI全双工通信：在发送一个字节数据的同时，会接收到一个字节数据。
  *          函数流程：等待发送缓冲区空 -> 写入发送数据 -> 等待接收缓冲区非空 -> 读取接收数据
  *          采用查询方式等待通信完成，超时时间取决于SPI时钟速度
  * @note 对于只发送不关心接收数据的场景（如写操作），返回值可以忽略；
  *       对于只接收不发送的场景（如读操作），需发送0xFF来产生时钟
  * @author He
  */
u8 Spi1_TransferData(u8 data)
{
    /* 等待发送缓冲区为空（TXE=1），确保可以写入新数据 */
    while(!SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE));  /* 查询等待发送标志位置位 */

    /* 向SPI1数据寄存器写入一个字节的数据，启动发送 */
    SPI_I2S_SendData(SPI1,data);

    /* 等待接收缓冲区非空（RXNE=1），确保数据已完整接收 */
    while(!SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE)); /* 查询等待接收标志位置位 */

    /* 从SPI1数据寄存器读取接收到的数据并返回 */
    return SPI_I2S_ReceiveData(SPI1);
}
