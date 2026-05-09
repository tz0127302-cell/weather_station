/**
 * @file usart1.c
 * @brief USART1串口驱动源文件
 * @details USART1用于控制台调试输出和数据通信，
 *          支持printf重定向，接收使用中断方式（RXNE+IDLE），
 *          接收到的数据直接写入W25Q64 Flash存储器
 * @author He
 * @date 2026-04-25
 */

#include "usart1.h"

/* USART1全局状态结构体，用于存储串口相关状态信息 */
U1 u1;

/**
 * @brief USART1串口初始化
 * @param baudrate: UART通信波特率（如115200）
 * @retval None
 * @details 配置PA9(TX)为复用推挽输出，PA10(RX)为浮空输入，
 *          设置串口参数（8位数据、1位停止位、无校验、无硬件流控），
 *          使能接收中断(RXNE)和空闲中断(IDLE)，
 *          接收中断将数据写入W25Q64，空闲中断用于检测数据包传输结束
 */
void USART1_Init(uint32_t baudrate)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    /* 使能USART1时钟和GPIOA时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    /* 配置PA9为TX发送引脚，复用推挽输出 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;     /* 复用推挽输出，由USART1外设控制 */
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* 配置PA10为RX接收引脚，浮空输入 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; /* 浮空输入，由外部设备驱动 */
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* 配置USART1通信参数 */
    USART_InitStructure.USART_BaudRate = baudrate;                        /* 设置波特率，如115200 */
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;           /* 8位数据位 */
    USART_InitStructure.USART_StopBits = USART_StopBits_1;                /* 1位停止位 */
    USART_InitStructure.USART_Parity = USART_Parity_No;                   /* 无校验位 */
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; /* 无硬件流控 */
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;      /* 使能接收和发送模式 */
    USART_Init(USART1, &USART_InitStructure);

    /* === 使能USART1中断 === */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);  /* 使能接收缓冲区非空中断（收到数据时触发） */
    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);  /* 使能空闲中断（总线空闲时触发，表示数据包接收完成） */

    /* === 配置NVIC中断优先级 === */
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;            /* USART1中断通道 */
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;    /* 抢占优先级为2 */
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;           /* 子优先级为2 */
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;              /* 使能该中断通道 */
    NVIC_Init(&NVIC_InitStructure);

    /* 使能USART1外设 */
    USART_Cmd(USART1, ENABLE);
}



/**
 * @brief USART1中断服务函数
 * @details 处理两种中断事件：
 *          - RXNE中断（接收缓冲区非空）：读取接收到的数据，写入W25Q64 Flash，
 *            地址递增，并设置file_flag=1表示正在接收文件数据
 *          - IDLE中断（总线空闲）：表示一帧数据接收完毕，
 *            当接收地址达到指定长度(0x001D3374)时，清除file_flag和transfer_flag
 * @note 通过读SR和DR寄存器清除IDLE中断标志位
 */
void USART1_IRQHandler(void)
{
    u8 data;
    static u32 addr = 0;  /* 静态地址变量，记录当前写入W25Q64的地址位置 */

    /* === RXNE中断处理：接收到一个字节数据 === */
    if(USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
    {
       /* 清除RXNE中断标志位 */
       USART_ClearITPendingBit(USART1, USART_IT_RXNE);
       /* 设置文件接收标志，表示正在接收文件数据 */
        file_flag = 1;

        /* 读取接收到的数据 */
        data = USART_ReceiveData(USART1);

        /* 将接收到的数据通过SPI写入W25Q64 Flash，地址自动递增 */
        W25Q64_CrossPageWrite(addr++,1,&data);
    }

    /* === IDLE中断处理：串口总线空闲，表示一帧数据接收完成 === */
    if(USART_GetITStatus(USART1, USART_IT_IDLE) == SET)
    {
        /* 清除IDLE中断标志位：先读SR寄存器再读DR寄存器（STM32标准清除方式） */
        USART1->SR;
        USART1->DR;

        /* 检查是否已接收到指定长度的数据（0x001D3374为目标文件大小） */
        if(addr >= 0x001D3374)
        {
            /* 清除文件接收标志和传输标志，表示文件传输完成 */
            file_flag = 0;
            transfer_flag = 0;
        }
    }
}








/**
 * @brief 通过USART1发送一个字节
 * @param data: 要发送的字节数据
 * @retval None
 * @details 查询等待发送数据寄存器空（TXE标志），然后写入数据寄存器
 */
void USART1_SendByte(uint8_t data)
{
    /* 等待发送缓冲区为空（TXE=1），确保可以发送新数据 */
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    /* 将数据写入USART1数据寄存器，启动发送 */
    USART_SendData(USART1, data);
}

/**
 * @brief 通过USART1发送字符串
 * @param str: 要发送的字符串指针（以'\0'结尾）
 * @retval None
 * @details 逐字节调用USART1_SendByte发送，直到遇到字符串结束符'\0'
 */
void USART1_SendString(char *str)
{
    /* 遍历字符串，直到遇到结束符'\0' */
    while(*str != '\0')
    {
        /* 逐个发送字符，并将指针后移 */
        USART1_SendByte(*str++);
    }
}



/**
 * @brief 重定向C库printf函数到USART1串口
 * @param ch: 要发送的字符
 * @param f: 文件指针（未使用，printf标准接口参数）
 * @retval int 返回发送的字符
 * @details 通过重写fputc函数，将对printf的调用映射到USART1硬件发送，
 *          从而在工程中可直接使用printf("...")通过串口输出调试信息
 */
int fputc(int ch, FILE *f)
{
    /* 调用串口发送函数发送字符 */
    USART1_SendByte((uint8_t)ch);
    return ch;  /* 返回发送的字符，符合C库标准 */
}
