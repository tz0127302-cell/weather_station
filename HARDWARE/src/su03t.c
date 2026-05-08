/**
 * @file su03t.c
 * @brief SU03T语音识别模块驱动源文件
 * @details 处理SU03T模块的UART接收与语音识别
 * @author He
 * @date 2026-04-25
 */

#include "su03t.h"


/**
  * @brief SU03T管脚初始化
  * @param  void
  * @retval void
  * @author He
    VOICE_TXD    ---    PC10   ---   复用推挽输出
    VOICE_RXD    ---    PC11   ---   浮空输入
    UART4初始化        
  
  */


void SU_03T_Config(void)
{
    
    //打开时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    //初始化VOICETXD
    GPIO_InitTypeDef GPIO_InitStructure={0};
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    //初始化VOICE_RXD
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    //打开UART4时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

    //初始化UART4
    USART_InitTypeDef USART_InitStructure={0};
    USART_InitStructure.USART_BaudRate = 9600;//设置波特率9600
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;// 8位数据位
    USART_InitStructure.USART_StopBits = USART_StopBits_1;// 1位停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;// 无校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;// 无硬件流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;// 使能接收和发送    
    USART_Init(UART4, &USART_InitStructure);

    //使能接收中断和空闲中断
    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);// 接收中断使能，当接收缓冲区非空时触发
    USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);// 空闲中断使能，当接收缓冲区为空且没有数据输入时触发
    
    
    //NVIC
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);


    // Enable UART4
    USART_Cmd(UART4, ENABLE);// 使能UART4

}

/**
  * @brief UART4中断服务函数
  * @param  void
  * @retval void
  * @author He       
  */
SU03T_Data su03t;
void UART4_IRQHandler(void)
{
    if(USART_GetITStatus(UART4, USART_IT_RXNE) == SET)
    {
        //清除接收中断标志位
        USART_ClearITPendingBit(UART4, USART_IT_RXNE);
        //user code
        su03t.buff[su03t.len++] = USART_ReceiveData(UART4);
    }
    if(USART_GetITStatus(UART4, USART_IT_IDLE) == SET)
    {
        //清除空闲中断标志位
        UART4->SR;
        UART4->DR;
        //user code

        su03t.len = 0;
        su03t.flag = 1;

        for(u8 i=0;i<4;i++)
        {
            printf("%02x ",su03t.buff[i]);            
        }
        printf("\r\n");


    }

}




/**
  * @brief 语音识别控制函数
  * @param  void
  * @retval void
  * @author He       
  */

void SU_03T_Control(void)
{
    if(su03t.flag == 1)
    {
        //处理数据
         switch(su03t.buff[0])
        {
            case 0x00: // 唤醒指令
                MY1680_Play(0x02, 0x06);
                break;
            case 0x01: // 开灯指令
                LED1_ON;
                LED2_ON;
                MY1680_Play(0x02, 0x07);
                break;
            case 0x02: // 关灯指令
                LED1_OFF;
                LED2_OFF;
                MY1680_Play(0x02, 0x08);
                break;
            default:
                break;
        }
        su03t.flag = 0;
    }

}







