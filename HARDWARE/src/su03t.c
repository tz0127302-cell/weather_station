/**
 * @file su03t.c
 * @brief SU03T语音识别模块驱动程序源文件
 * @details 实现SU03T模块的UART通信接口，包括模块初始化、
 *         中断接收和语音指令解析处理。使用STM32的UART4
 *         与SU03T通信，通过RXNE和IDLE中断接收不定长数据帧。
 * @author He
 * @date 2026-04-25
 */

#include "su03t.h"


/**
  * @brief SU03T引脚及UART4初始化
  * @param  void
  * @retval void
  * @note 引脚连接：
  *       VOICE_TXD   ---   PC10   ---   SU03T发送 -> STM32接收
  *       VOICE_RXD   ---   PC11   ---   STM32发送 -> SU03T接收
  * @details 初始化流程：
  *         1. 使能GPIOC时钟
  *         2. 配置PC10为复用推挽输出（UART4_TX）
  *         3. 配置PC11为浮空输入（UART4_RX）
  *         4. 使能UART4时钟，配置9600-8-N-1串口参数
  *         5. 使能接收中断(RXNE)和空闲中断(IDLE)
  *         6. 配置NVIC中断优先级（抢占优先级2，子优先级2）
  *         7. 使能UART4
  */

void SU_03T_Config(void)
{

    /* 使能GPIOC时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    /* 初始化VOICE_TXD (PC10) --- 复用推挽输出 */
    GPIO_InitTypeDef GPIO_InitStructure={0};
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;      /* 复用推挽输出 */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* 初始化VOICE_RXD (PC11) --- 浮空输入 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; /* 浮空输入 */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* 使能UART4时钟（UART4挂载在APB1总线上） */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

    /* 初始化UART4串口参数 */
    USART_InitTypeDef USART_InitStructure={0};
    USART_InitStructure.USART_BaudRate = 9600;              /* 波特率：9600 */
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;  /* 数据位：8位 */
    USART_InitStructure.USART_StopBits = USART_StopBits_1;       /* 停止位：1位 */
    USART_InitStructure.USART_Parity = USART_Parity_No;         /* 无校验位 */
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; /* 无硬件流控制 */
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;  /* 使能接收和发送 */
    USART_Init(UART4, &USART_InitStructure);

    /* 使能接收中断和空闲中断 */
    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);  /* 接收中断使能：每收到一个字节触发 */
    USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);  /* 空闲中断使能：接收完一帧数据（总线空闲）时触发 */


    /* 配置NVIC嵌套向量中断控制器 */
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;             /* UART4中断通道 */
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;    /* 抢占优先级：2 */
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;           /* 子优先级：2 */
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;              /* 使能中断通道 */
    NVIC_Init(&NVIC_InitStructure);


    /* 使能UART4 */
    USART_Cmd(UART4, ENABLE);

}

/**
  * @brief UART4中断服务函数
  * @param  void
  * @retval void
  * @details 处理两种中断事件：
  *         1. RXNE（接收数据寄存器非空中断）：
  *            每收到一个字节触发，将数据存入缓冲区并递增长度计数器。
  *         2. IDLE（空闲中断）：
  *            当接收完一帧数据（总线空闲超过一个字符时间）时触发，
  *            设置标志位通知主循环处理数据。
  *            IDLE中断的清除方式：先读SR寄存器，再读DR寄存器。
  */
SU03T_Data su03t;                    /* SU03T数据接收结构体全局变量 */
void UART4_IRQHandler(void)
{
    /* 处理RXNE中断：接收到一个字节 */
    if(USART_GetITStatus(UART4, USART_IT_RXNE) == SET)
    {
        /* 清除接收中断标志位 */
        USART_ClearITPendingBit(UART4, USART_IT_RXNE);
        /* 用户代码：读取接收到的数据并存入缓冲区 */
        su03t.buff[su03t.len++] = USART_ReceiveData(UART4);
        /* 注意：此处未做缓冲区溢出保护，需确保buff足够大 */
    }
    /* 处理IDLE中断：一帧数据接收完毕（总线空闲） */
    if(USART_GetITStatus(UART4, USART_IT_IDLE) == SET)
    {
        /* 清除空闲中断标志位：先读SR再读DR（STM32标准清除方式） */
        UART4->SR;
        UART4->DR;
        /* 用户代码：帧接收完成，设置标志 */

        su03t.len = 0;          /* 重置长度计数器，准备接收下一帧 */
        su03t.flag = 1;         /* 设置接收完成标志，通知主循环处理 */

        /* 打印接收到的数据（前4字节），用于调试 */
        for(u8 i=0;i<4;i++)
        {
            printf("%02x ",su03t.buff[i]);    /* 以十六进制格式打印 */
        }
        printf("\r\n");


    }

}





/**
  * @brief 语音识别控制处理函数
  * @param  void
  * @retval void
  * @details 在主循环中调用，检查SU03T识别到的语音指令。
  *         当接收完成标志置位时，解析数据帧的第一个字节：
  *         0x00 --- 开灯指令：播放"开灯"音频
  *         0x01 --- 亮灯指令：点亮LED1和LED2，播放"已亮灯"音频
  *         0x02 --- 关灯指令：熄灭LED1和LED2，播放"已关灯"音频
  *         处理完成后清除标志位，等待下一次指令。
  * @note SU03T每次识别到语音后，会通过UART发送固定格式的数据帧，
  *       第一个字节为指令码，对应不同的语音命令。
  */

void SU_03T_Control(void)
{
    if(su03t.flag == 1)              /* 检查是否有新的语音识别数据到达 */
    {
        /* 根据指令码执行对应操作 */
         switch(su03t.buff[0])
        {
            case 0x00:              /* 开灯指令 */
                MY1680_Play(0x02, 0x06);   /* 播放"开灯"语音提示 */
                break;
            case 0x01:              /* 亮灯指令 */
                LED1_ON;            /* LED1点亮 */
                LED2_ON;            /* LED2点亮 */
                MY1680_Play(0x02, 0x07);   /* 播放"已亮灯"语音提示 */
                break;
            case 0x02:              /* 关灯指令 */
                LED1_OFF;           /* LED1熄灭 */
                LED2_OFF;           /* LED2熄灭 */
                MY1680_Play(0x02, 0x08);   /* 播放"已关灯"语音提示 */
                break;
            default:
                /* 未识别的指令码，不做处理 */
                break;
        }
        su03t.flag = 0;             /* 清除标志位，准备接收下一条指令 */
    }

}







