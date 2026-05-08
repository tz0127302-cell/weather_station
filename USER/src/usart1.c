/**
 * @file usart1.c
 * @brief USART1驱动源文件
 * @details USART1串口控制和调试输出函数
 * @author He
 * @date 2026-04-25
 */

#include "usart1.h"

U1 u1;

/**
 * @brief Initialize USART1
 * @param baudrate: UART baud rate
 * @retval None
 */
void USART1_Init(uint32_t baudrate)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    // Enable clocks
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    // Configure PA9 as TX pin
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Configure PA10 as RX pin
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Configure USART1
    USART_InitStructure.USART_BaudRate = baudrate;//设置波特率115200
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;// 8位数据位
    USART_InitStructure.USART_StopBits = USART_StopBits_1;// 1位停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;// 无校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;// 无硬件流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;// 使能接收和发送    
    USART_Init(USART1, &USART_InitStructure);

    //使能接收中断和空闲中断
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);// 接收中断使能，当接收缓冲区非空时触发
    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);// 空闲中断使能，当接收缓冲区为空且没有数据输入时触发
    
    
    //NVIC
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);


    // Enable USART1
    USART_Cmd(USART1, ENABLE);// 使能USART1

}



void USART1_IRQHandler(void)
{
    u8 data;
    static u32 addr = 0;
    if(USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
    {
       //清除标志位
       USART_ClearITPendingBit(USART1, USART_IT_RXNE);
       //user handle
        file_flag = 1;
        
        data = USART_ReceiveData(USART1);
        
        W25Q64_CrossPageWrite(addr++,1,&data);
       
    }

    if(USART_GetITStatus(USART1, USART_IT_IDLE) == SET)
    {
        //清除标志位
        USART1->SR;
        USART1->DR;
         //user handle
       
        if(addr >= 0x001D3374)
        {
            file_flag = 0;
            transfer_flag = 0;
            
        }


    }
    
}









/**
 * @brief Send a byte via USART1
 * @param data: Data to be sent
 * @retval None
 */
void USART1_SendByte(uint8_t data)
{
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, data);
    
}

/**
 * @brief Send a string via USART1
 * @param str: String to be sent
 * @retval None
 */
void USART1_SendString(char *str)
{
    while(*str != '\0')
    {
        USART1_SendByte(*str++);
    }
}



/**
 * @brief Retargets the C library printf function to the USART
 * @param ch: Character to send
 * @param f: File pointer (unused)
 * @retval Character sent
 */
int fputc(int ch, FILE *f)
{
    USART1_SendByte((uint8_t)ch);
    return ch;
}