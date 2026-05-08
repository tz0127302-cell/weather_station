/**
 * @file my1680.c
 * @brief MY1680语音合成模块驱动源文件
 * @details 处理MY1680模块的UART控制接口
 * @author He
 * @date 2026-04-25
 */

#include "my1680.h"


/**
  * @brief UART5管脚初始化
  * @param  void
  * @retval void
  * @author He
    MY1680_BUSY     ---    PB8     ---    浮空输入
    UART5_RX       ---     PD2    ---    浮空输入
    UART5_TX       ---     PC12  ---   复用推挽输出
  
  */


void MY1680_Config(void)
{
    
    //打开时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

    //初始化UART5_TX
    GPIO_InitTypeDef GPIO_InitStructure={0};
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    //初始化UART5_RX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    //初始化MY1680_BUSY
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    //打开UART4时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);

    //初始化UART5
    USART_InitTypeDef USART_InitStructure={0};
    USART_InitStructure.USART_BaudRate = 9600;//设置波特率9600
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;// 8位数据位
    USART_InitStructure.USART_StopBits = USART_StopBits_1;// 1位停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;// 无校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;// 无硬件流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;// 使能接收和发送    
    USART_Init(UART5, &USART_InitStructure);

  
    // Enable UART5
    USART_Cmd(UART5, ENABLE);// 使能UART5

}


/**
 * @brief Send a byte via USART1
 * @param data: Data to be sent
 * @retval None
 */
void UART5_SendByte(uint8_t data)
{
    while(USART_GetFlagStatus(UART5, USART_FLAG_TXE) == RESET);
    USART_SendData(UART5, data);
    
}


/**
 * @brief Send a byte via USART1
 * @param data: Data to be sent
 * @retval None
 */

u8 play[5] = {0x7E,0x03,0x11,0x12,0xEF};//播放语音
u8 stop[5] = {0x7E,0x03,0x1E,0x1D,0xEF};//停止语音

 void UART5_SendBuff(uint8_t *buff, uint8_t len)
{
    for(uint8_t i=0;i<len;i++)
    {
        UART5_SendByte(buff[i]);
    }
}


/**
  * @brief 播放指定音频文件
  * @param  
  *         u8 dir_name: 音频文件目录名
  *         u8 file_name: 音频文件名
  * @retval void
  * @author He
  
  */

void MY1680_Play(u8 dir_name, u8 file_name)
{
    u8 buff[7] = {0x7E,0x05,0x42,dir_name,file_name,0x00,0xEF};
    //校验码 = 长度^操作码^参数
    buff[5] = buff[1] ^ buff[2] ^ buff[3] ^ buff[4];
    printf("播放音频文件:0x%02x\r\n",buff[5]);
    //发送数据
    UART5_SendBuff(buff,7);

}





