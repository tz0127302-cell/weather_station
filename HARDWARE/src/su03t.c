/**
 * @file su03t.c
 * @brief SU03T语音识别模块驱动源文件
 * @details 实现SU03T模块的UART通信接口，包括模块初始化、
 *         中断接收和语音指令处理。使用STM32的UART4与SU03T通信。
 * @author He
 * @date 2026-04-25
 */

#include "su03t.h"


/**
  * @brief SU03T引脚及UART4初始化
  */
void SU_03T_Config(void)
{
    /* 使能GPIOC时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    /* 初始化VOICE_TXD (PC10) --- 复用推挽输出 */
    GPIO_InitTypeDef GPIO_InitStructure={0};
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* 初始化VOICE_RXD (PC11) --- 浮空输入 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* 使能UART4时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

    /* 初始化UART4: 9600-8-N-1 */
    USART_InitTypeDef USART_InitStructure={0};
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(UART4, &USART_InitStructure);

    /* 使能接收中断和空闲中断 */
    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
    USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);

    /* 配置NVIC */
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* 使能UART4 */
    USART_Cmd(UART4, ENABLE);
}

/**
  * @brief UART4中断服务函数
  * @note  RXNE: 每收到一个字节触发，存入缓冲区
  *        IDLE: 总线空闲时触发，设置flag=1通知主循环
  */
volatile SU03T_Data su03t;
void UART4_IRQHandler(void)
{
    /* 处理RXNE中断 */
    if(USART_GetITStatus(UART4, USART_IT_RXNE) == SET)
    {
        USART_ClearITPendingBit(UART4, USART_IT_RXNE);
        if(su03t.len < 4)
        {
            su03t.buff[su03t.len] = USART_ReceiveData(UART4);
            su03t.len++;
        }
        else
        {
            /* 缓冲区满时必须读DR寄存器以清除RXNE标志位，否则中断反复触发导致死循环 */
            volatile u8 dummy = USART_ReceiveData(UART4);
            (void)dummy;   /* 消除"变量未使用"的编译警告 */
        }

    }
    /* 处理IDLE中断: 一帧接收完毕 */
    if(USART_GetITStatus(UART4, USART_IT_IDLE) == SET)
    {
        UART4->SR;
        UART4->DR;

        su03t.len = 0;
        su03t.flag = 1;

        /* DEBUG: 打印收到的指令, 标注是否在播报期间收到 */
        printf("[SU03T] RX: %02x %02x %02x %02x (busy=%d)\r\n",
               su03t.buff[0], su03t.buff[1], su03t.buff[2], su03t.buff[3],
               voice_busy);
    }
}


/* ======================== 音乐播放状态 ======================== */
static u8 current_track = 0;  /* 当前播放的音乐轨道索引 */
static const u8 total_tracks = 2;  /* 总音乐轨道数 */

/* ======================== 语音播报保护标志 ========================
 * voice_busy:  1=正在播报
 * voice_abort: 1=需要中止当前播报
 * volatile: WeatherTask写入, KeyTask读取, 跨任务共享
 *==================================================================*/
volatile u8 voice_busy = 0;  /* 播报忙标志: 1=正在播报 0=空闲, 忙时只响应暂停/停止 */
volatile u8 voice_abort = 0;  /* 播报中止标志: 1=需要中止当前播报, 由暂停/停止指令设置 */
static u8 voice_cooldown = 0;     /* 播报结束后的冷却计数(单位: 20ms), 期间丢弃所有指令 */

/**
 * @brief 语音识别控制处理函数
 * @details 由KeyTask每20ms轮询一次。
 *         - cooldown期间: 丢弃所有指令
 *         - voice_busy期间: 只允许0x08/0x09/0x0A/0x0B通过
 */
void SU_03T_Control(void)
{
    /* 播报结束后的冷却期：每收到指令重置倒计时，连续2秒无指令才结束 */
    if (voice_cooldown > 0)
    {
        voice_cooldown--;
        if (su03t.flag == 1)
        {
            printf("[SU03T] COOLDOWN(%d): cmd=0x%02X discarded\r\n",
                   voice_cooldown, su03t.buff[0]);
            su03t.flag = 0;
            voice_cooldown = 100;    /* 收到指令重置冷却，确保连续2秒无数据才退出 */
        }
        if (voice_cooldown == 0)
        {
            voice_busy = 0;
            printf("[SU03T] cooldown end, voice ready\r\n");
        }
        return;
    }

    if(su03t.flag == 1)
    {
        u8 cmd = su03t.buff[0];
        u8 cmd_buf;

        /* 播报忙时的指令过滤 */
        if (voice_busy)
        {
            if (cmd == 0x0A || cmd == 0x0B)
            {
                printf("[SU03T] BUSY: cmd=0x%02X ALLOWED (stop/pause)\r\n", cmd);
                voice_abort = 1;
                voice_busy = 0;
                UART5_SendBuff(stop, 5);
            }
            else if (cmd == 0x08 || cmd == 0x09)
            {
                printf("[SU03T] BUSY: cmd=0x%02X ALLOWED (prev/next)\r\n", cmd);
                /* 放行，继续执行下方switch */
            }
            else
            {
                printf("[SU03T] BUSY: cmd=0x%02X BLOCKED\r\n", cmd);
                su03t.flag = 0;
                return;
            }
        }

        switch(cmd)
        {
            case 0x00:  /* 唤醒 */
                printf("[SU03T] CMD: wakeup\r\n");
                MY1680_Play(0x02, 0x02);
                break;

            case 0x01:  /* 开灯 */
                printf("[SU03T] CMD: light ON\r\n");
                LED1_ON;
                LED2_ON;
                MY1680_Play(0x02, 0x07);
                break;

            case 0x02:  /* 关灯 */
                printf("[SU03T] CMD: light OFF\r\n");
                LED1_OFF;
                LED2_OFF;
                MY1680_Play(0x02, 0x08);
                break;

            case 0x03:  /* 播报天气 */
                printf("[SU03T] CMD: weather broadcast\r\n");
                cmd_buf = CMD_WEATHER_VOICE;
                xQueueSend(weatherCmdQueue, &cmd_buf, 0);
                break;

            case 0x04:  /* 播报时间 */
                printf("[SU03T] CMD: report time\r\n");
                voice_busy = 1;
                voice_abort = 0;
                Voice_ReportTime();
                voice_busy = 0;
                break;

            case 0x05:  /* 播报室内温度 */
                printf("[SU03T] CMD: report indoor temp\r\n");
                voice_busy = 1;
                voice_abort = 0;
                Voice_ReportIndoorTemp();
                voice_busy = 0;
                break;

            case 0x06:  /* 播报室内湿度 */
                printf("[SU03T] CMD: report indoor hum\r\n");
                voice_busy = 1;
                voice_abort = 0;
                Voice_ReportIndoorHum();
                voice_busy = 0;
                break;

            case 0x07:  /* 播放音乐 */
                printf("[SU03T] CMD: play music track=%d\r\n", current_track);
                MY1680_Play(0x01, current_track);
                voice_busy = 1;
                voice_abort = 0;
                break;

            case 0x08:  /* 上一首 */
                printf("[SU03T] CMD: prev track\r\n");
                if(current_track > 0)
                    current_track--;
                else
                    current_track = total_tracks - 1;
                UART5_SendBuff(stop, 5);
                MY1680_Play(0x01, current_track);
                voice_busy = 1;
                voice_abort = 0;
                break;

            case 0x09:  /* 下一首 */
                printf("[SU03T] CMD: next track\r\n");
                current_track = (current_track + 1) % total_tracks;
                UART5_SendBuff(stop, 5);
                MY1680_Play(0x01, current_track);
                voice_busy = 1;
                voice_abort = 0;
                break;

            case 0x0A:  /* 暂停 */
                printf("[SU03T] CMD: pause\r\n");
                UART5_SendBuff(stop, 5);
                voice_busy = 0;
                break;

            case 0x0B:  /* 停止 */
                printf("[SU03T] CMD: stop\r\n");
                UART5_SendBuff(stop, 5);
                voice_busy = 0;
                break;

            default:
                printf("[SU03T] CMD: unknown 0x%02X\r\n", cmd);
                break;
        }
        su03t.flag = 0;
    }
}

/**
 * @brief 禁用SU-03T的UART4中断（播报期间静默）
 * @note  播报天气/时间/温湿度前调用。
 *        禁用期间SU-03T的数据帧不会触发ISR，从硬件层面杜绝误触发。
 */
void SU03T_DisableIRQ(void)
{
    printf("[SU03T] >>> IRQ DISABLED (mute) <<<\r\n");
    USART_ITConfig(UART4, USART_IT_RXNE, DISABLE);
    USART_ITConfig(UART4, USART_IT_IDLE, DISABLE);
}

/**
 * @brief 恢复SU-03T的UART4中断（播报结束后）
 * @note  清空播放期间积累的残留数据、pending标志和UART错误标志。
 */
void SU03T_EnableIRQ(void)
{
    printf("[SU03T] <<< IRQ ENABLED (unmute), cooldown 2s >>>\r\n");
    su03t.len = 0;
    su03t.flag = 0;

    USART_ClearITPendingBit(UART4, USART_IT_RXNE);
    USART_ClearITPendingBit(UART4, USART_IT_IDLE);

    (void)UART4->SR;
    (void)UART4->DR;

    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
    USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);

    voice_cooldown = 300;    /* 300 * 20ms = 6秒初始冷却，足够覆盖SU-03T延迟 */
}
