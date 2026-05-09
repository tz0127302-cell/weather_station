/**
 * @file my1680.c
 * @brief MY1680语音合成模块驱动程序源文件
 * @details 实现MY1680模块的UART控制接口，包括初始化、
 *         数据发送和指定音频文件播放功能。
 *         使用STM32的UART5与MY1680通信。
 * @author He
 * @date 2026-04-25
 */

#include "my1680.h"


/**
  * @brief UART5引脚及MY1680模块初始化
  * @param  void
  * @retval void
  * @note 引脚连接：
  *       MY1680_BUSY   ---   PB8     ---   忙状态检测引脚（输入）
  *       UART5_RX      ---   PD2     ---   接收引脚（输入）
  *       UART5_TX      ---   PC12    ---   发送引脚（输出）
  * @details 初始化流程：
  *         1. 使能GPIOB/GPIOC/GPIOD时钟
  *         2. 配置PC12为复用推挽输出（UART5_TX）
  *         3. 配置PD2为浮空输入（UART5_RX）
  *         4. 配置PB8为浮空输入（MY1680_BUSY状态检测）
  *         5. 使能UART5时钟，配置9600-8-N-1串口参数
  *         6. 使能UART5
  */

void MY1680_Config(void)
{

    /* 使能GPIOB、GPIOC、GPIOD时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

    /* 初始化UART5_TX (PC12) --- 复用推挽输出 */
    GPIO_InitTypeDef GPIO_InitStructure={0};
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;      /* 复用推挽输出（由片上外设控制） */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* 初始化UART5_RX (PD2) --- 浮空输入 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; /* 浮空输入，由外部驱动 */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    /* 初始化MY1680_BUSY (PB8) --- 浮空输入 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; /* 检测MY1680是否忙 */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* 使能UART5时钟（UART5挂载在APB1总线上） */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);

    /* 初始化UART5串口参数 */
    USART_InitTypeDef USART_InitStructure={0};
    USART_InitStructure.USART_BaudRate = 9600;              /* 波特率：9600 */
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;  /* 数据位：8位 */
    USART_InitStructure.USART_StopBits = USART_StopBits_1;       /* 停止位：1位 */
    USART_InitStructure.USART_Parity = USART_Parity_No;         /* 无校验位 */
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; /* 无硬件流控制 */
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;  /* 使能接收和发送 */
    USART_Init(UART5, &USART_InitStructure);


    /* 使能UART5 */
    USART_Cmd(UART5, ENABLE);

}


/**
 * @brief 通过UART5发送一个字节
 * @param data: 要发送的字节数据
 * @retval None
 * @details 等待发送数据寄存器为空（TXE标志位置位），
 *         然后将数据写入发送数据寄存器，由UART5硬件
 *         自动完成并串转换和移位输出。
 */
void UART5_SendByte(uint8_t data)
{
    /* 等待上一次发送完成：TXE（发送数据寄存器空）标志位为SET */
    while(USART_GetFlagStatus(UART5, USART_FLAG_TXE) == RESET);
    USART_SendData(UART5, data);  /* 将数据写入发送寄存器 */

}


/**
 * @brief 通过UART5发送多个字节（数据缓冲区）
 * @param data: 要发送的数据缓冲区指针
 * @param len: 要发送的字节数
 * @retval None
 * @details 循环调用UART5_SendByte()逐个字节发送。
 */

u8 play[5] = {0x7E,0x03,0x11,0x12,0xEF};   /* MY1680播放指令：0x7E 0x03 0x11 0x12 0xEF */
u8 stop[5] = {0x7E,0x03,0x1E,0x1D,0xEF};   /* MY1680停止指令：0x7E 0x03 0x1E 0x1D 0xEF */

void UART5_SendBuff(uint8_t *buff, uint8_t len)
{
    for(uint8_t i=0;i<len;i++)
    {
        UART5_SendByte(buff[i]);  /* 逐字节发送 */
    }
}


/**
  * @brief 播放指定音频文件
  * @param  u8 dir_name: 音频文件目录号（对应SD卡中的文件夹编号）
  *         u8 file_name: 音频文件名（对应文件夹中的文件编号）
  * @retval void
  * @details MY1680指令格式：
  *         帧头(0x7E) + 数据长度(0x05) + 命令(0x42指定播放) +
  *         目录号 + 文件名 + 校验和 + 帧尾(0xEF)
  *         校验和 = 数据长度 ^ 命令 ^ 目录号 ^ 文件名
  *         （异或校验，不含帧头和帧尾）
  * @note 发送前需检查MY1680_BUSY引脚状态，确保模块不忙。
  *       play[5]和stop[5]为预定义的全局指令数组，可直接调用。
  */

void MY1680_Play(u8 dir_name, u8 file_name)
{
    u8 buff[7] = {0x7E,0x05,0x42,dir_name,file_name,0x00,0xEF};
    /* 计算校验和：对数据长度、命令、目录号和文件名进行异或运算 */
    buff[5] = buff[1] ^ buff[2] ^ buff[3] ^ buff[4];
    printf("播放音频文件,校验和:0x%02x\r\n",buff[5]);  /* 打印校验和用于调试 */
    /* 发送完整指令帧 */
    UART5_SendBuff(buff,7);

}





