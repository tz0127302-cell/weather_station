/**
 * @file wifi.c
 * @brief WIFI模块通信驱动源文件
 * @details 通过USART3与ESP-12F(WIFI模块)通信，使用AT指令集控制
 *         主要功能包括：NTP时间同步、天气API数据获取
 * @note   ESP-12F基于ESP8266，通过UART AT指令控制
 *         本文件使用USART3，波特率115200
 * @author He
 * @date 2026-04-25
 */

#include "wifi.h"

/* 全局变量定义 */
u32 timestamp = 0;          /* NTP时间戳(Unix时间戳，秒数) */
u8 ntp_flag = 0;            /* NTP同步完成标志位：0-未完成，1-已完成 */

/**
  * @brief USART3初始化（ESP-12F WiFi模块通信接口）
  * @param void
  * @retval void
  * @author He
  * @note   硬件引脚连接：
  *         USART3_TX    ---    PB10    （STM32发送 -> ESP-12F接收）
  *         USART3_RX    ---    PB11    （STM32接收 <- ESP-12F发送）
  *         WIFI_Enable  ---    PC9     （模块使能控制引脚，高电平使能）
  *
  *         配置参数：波特率115200，8位数据位，1位停止位，无校验，无硬件流控
  *         使能接收中断(RXNE)和空闲中断(IDLE)
  *         空闲中断用于检测一帧数据接收完毕
  */

void Usart3_Config(void)
{
    /* 使能GPIOB、GPIOC和USART3时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC,ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);


    GPIO_InitTypeDef GPIO_InitStructure={0};
    /* 配置PC9为推挽输出，用于控制WIFI模块使能 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIOC->ODR |= 1 << 9;   /* 拉高PC9，使能WIFI模块 */

    /* 配置PB10为复用推挽输出(USART3_TX) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);


    /* 配置PB11为浮空输入(USART3_RX) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);



    // USART3初始化配置
    USART_InitTypeDef USART_InitStruct={0};
    USART_InitStruct.USART_BaudRate = 115200;           /* 波特率115200 */
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  /* 无硬件流控(RTS/CTS) */
    USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;  /* 使能发送和接收 */
    USART_InitStruct.USART_Parity = USART_Parity_No;    /* 无校验位 */
    USART_InitStruct.USART_StopBits = USART_StopBits_1; /* 1位停止位 */
    USART_InitStruct.USART_WordLength = USART_WordLength_8b; /* 8位数据长度 */
    USART_Init(USART3,&USART_InitStruct);



    // 中断配置
    USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);   /* 使能接收缓冲区非空中断(接收到数据时触发) */
    USART_ITConfig(USART3,USART_IT_IDLE,ENABLE);   /* 使能空闲线中断(一帧数据接收完毕时触发) */

    // NVIC中断优先级设置
    NVIC_SetPriority(USART3_IRQn,NVIC_EncodePriority(5, 1, 1));
    NVIC_EnableIRQ(USART3_IRQn);

    // 使能USART3外设
    USART_Cmd(USART3,ENABLE);

}

/**
  * @brief USART3中断处理函数
  * @param  void
  * @retval void
  * @author He
  * @note   处理两种中断：
  *         1. RXNE中断：串口接收到1字节数据，存入环形缓冲区u3.buff
  *         2. IDLE中断：总线空闲，表示一帧数据接收完成
  *            设置帧长度和标志位，供主循环处理
  *         接收数据存储在全局结构体u3中，包括：
  *         - buff: 数据缓冲区
  *         - len: 当前累计接收长度
  *         - frame_len: 完整一帧的长度(IDLE时更新)
  *         - flag: 帧完成标志，主循环检测到后处理数据
  */
U3 u3;                      /* USART3接收数据结构体 */
void USART3_IRQHandler(void)
{

    if(USART_GetITStatus(USART3,USART_IT_RXNE))
    {
        USART_ClearITPendingBit(USART3,USART_IT_RXNE);
        // 用户处理：接收1字节数据存入缓冲区

        if (u3.len < sizeof(u3.buff))
            u3.buff[u3.len++] = USART_ReceiveData(USART3);

    }
    if(USART_GetITStatus(USART3,USART_IT_IDLE))
    {
        /* 空闲中断产生条件：收到一帧完整数据后总线空闲 */
        USART3->SR;          /* 先读SR寄存器(清除IDLE标志的步骤1) */
        USART3->DR;          /* 再读DR寄存器(清除IDLE标志的步骤2) */
        // 用户处理：标记一帧数据接收完成

        u3.buff[u3.len] = '\0';  /* 字符串结束符 */
        u3.frame_len = u3.len;   /* 记录本帧数据长度 */
        u3.len = 0;              /* 重置接收计数，准备接收下一帧 */
        u3.flag = 1;             /* 设置帧完成标志，通知主循环处理 */


    }


}


/**
  * @brief USART3发送1字节数据
  * @param u8 data 要发送的1字节数据
  * @retval void
  * @author He
  * @note   采用查询方式发送：等待发送数据寄存器为空(TXE标志位为1)
  *         然后写入数据寄存器，由硬件自动完成移位发送
  */

void usart3_send_byte(u8 data)
{

    while(!USART_GetFlagStatus(USART3,USART_FLAG_TXE));  /* 等待发送缓冲区为空 */
    USART_SendData(USART3, data);

}




/**
  * @brief USART3发送字符串（支持多字节数据发送）
  * @param u8 *str  指向要发送的字符串，以'\0'结尾
  * @retval void
  * @author He
  * @note   逐字节调用usart3_send_byte发送，直到遇到字符串结束符'\0'
  *         发送内容包括AT指令和HTTP请求等
  */

void usart3_send_str(char *str)
{

    while(*str != '\0')
    {
       usart3_send_byte(*str);
       str++;
    }


}



/**
  * @brief 向ESP-12F发送AT指令，并等待返回"OK"
  * @param
  *         char *cmd    要发送给ESP-12F的AT指令字符串
  *         u32 time_out 等待超时时间(毫秒)
  * @retval u8    0: 成功(收到OK响应)      1: 超时失败
  * @author He
  * @note   发送AT指令后，通过检测u3.flag判断是否收到ESP-12F的响应
  *         在响应中搜索"OK"字符串来判断指令是否执行成功
  *         如果超时仍未收到"OK"，函数返回1表示失败
  *         注意：ESP-12F的AT指令响应可能包含多行，末尾出现"OK\r\n"表示成功
  */

u8 Wifi_SendCmd(char *cmd,u32 time_out)
{
    u32 count = 0;
    // 发送AT指令
    usart3_send_str(cmd);
    delay_ms(300);

    // 等待ESP-12F回复，检测是否包含"OK"
    while(1)
    {
        // 等待数据接收完成(等待u3.flag被中断置位)
        while(!u3.flag)
        {
          count++;
          delay_ms(1);
          if(count >= time_out)   /* 超时检测 */
          {
              u3.len = 0;         /* 清空接收计数器 */
              return 1;           /* 返回超时失败 */
          }

        }
        // U3成功接收到ESP-12F的一帧回复数据，清除标志位准备下一帧
        u3.flag = 0;
        // 判断接收缓冲区中是否包含"OK"字符串
        if(strstr((const char *)u3.buff,"OK") != NULL)
        {
            return 0;            /* 指令执行成功 */
        }

    }


}







/**
  * @brief ESP-12F WiFi模块初始化及NTP时间同步
  * @param  void
  * @retval u8    0: 全部初始化成功    非0: 失败(返回错误步骤编号)
  * @author He
  * @note   初始化流程：
  *         1. 初始化USART3
  *         2. 发送"+++"退出可能的透传模式
  *         3. 发送"AT"确认模块响应
  *         4. 恢复出厂设置(AT+RESTORE)
  *         5. 设置Station模式(AT+CWMODE=1)
  *         6. 连接WiFi热点(AT+CWJAP)
  *         7. 设置透传模式(AT+CIPMODE=1)
  *         8. 建立TCP连接到NTP服务器(api.k780.com:80)
  *         9. 进入发送模式(AT+CIPSEND)
  *         10. 发送NTP HTTP GET请求
  *         11. 从响应中解析时间戳
  *
  *         注意：WiFi名称和密码硬编码为"WIFI-6G"/"11111111"
  *         NTP API key为固定值，需根据实际服务情况更新
  */

u8 Wifi_NtpInit(void)
{
    u8 ret = 0;

    // 初始化USART3串口
    Usart3_Config();

    // 发送"+++"退出透传模式(如果模块处于透传状态)
    usart3_send_str("+++");
    delay_ms(300);
    // 发送"AT\r\n"检测模块是否正常响应
    usart3_send_str("AT\r\n");
    delay_ms(300);
    // 发送恢复出厂设置指令(AT+RESTORE)，清除之前的配置
    usart3_send_str("AT+RESTORE\r\n");
    delay_ms(1000);
    // 设置WiFi模式为Station(客户端)模式
    ret = Wifi_SendCmd("AT+CWMODE=1\r\n",2000);
    if(ret)
    {
        printf("客户端模式失败\r\n");
        return 1;
    }
    printf("客户端模式成功\r\n");
    // 连接指定的WiFi热点(AP)
    MY1680_Play(0x02, 0x03); /*正在连接网络*/
    ret = Wifi_SendCmd("AT+CWJAP=\"WIFI-6G\",\"11111111\"\r\n",15000);
    if(ret)
    {
        printf("连接AP失败\r\n");
        MY1680_Play(0x02, 0x05); /*连接网络失败*/
        return 2;
    }
    MY1680_Play(0x02, 0x04); /*连接网络成功*/
    printf("连接AP成功\r\n");
    // 开启透传模式，简化后续数据传输(无需每次发送AT+CIPSEND)
    ret = Wifi_SendCmd("AT+CIPMODE=1\r\n",2000);
    if(ret)
    {
        printf("设置透传模式失败\r\n");
        return 3;
    }
    printf("设置透传模式成功\r\n");
    // 建立TCP连接到NTP时间服务器(api.k780.com:80)
    ret = Wifi_SendCmd("AT+CIPSTART=\"TCP\",\"api.k780.com\",80\r\n",2000);
    if(ret)
    {
        printf("连接NTP服务器失败\r\n");
        return 4;
    }
    printf("连接NTP服务器成功\r\n");
    // 进入发送模式(透传模式下发送数据的起始命令)
     ret = Wifi_SendCmd("AT+CIPSEND\r\n",2000);
    if(ret)
    {
        printf("进入发送模式失败\r\n");
        return 5;
    }
    printf("进入发送模式成功\r\n");
    // 发送NTP HTTP GET请求，获取Unix时间戳
    usart3_send_str("GET http://api.k780.com:88/?app=life.time&appkey=10003&sign=b59bc3ef6191eb9f747dd4e83c99f2a4&format=json&HTTP/1.1\r\n");

    delay_ms(1000);

    // 从响应数据中解析时间戳，跳过非数字字符
    sscanf(&u3.buff[20],"%*[^0123456789]%d",&timestamp);
    printf("timestamp:%d\r\n",timestamp);

    ntp_flag = 1;        /* 设置NTP同步完成标志 */

    return ret;

}


/**
  * @brief  WiFi模块获取天气数据（通过HTTP API）
  * @param  city_code: 城市代码字符串（如"东莞"对应的代码）
  * @param  response:  存放HTTP响应数据的缓冲区
  * @param  max_len:   缓冲区最大长度
  * @retval u8  0: 成功  1: 设置透传模式失败  2: TCP连接失败
  *              3: CIPSEND超时  4: 响应超时
  * @author He
  * @date 2026-05-06
  * @note   工作流程：
  *         1. 发送"+++"退出透传模式
  *         2. 关闭可能存在的旧TCP连接
  *         3. 重新设置透传模式
  *         4. 建立TCP连接到天气API服务器(api.yytianqi.com:80)
  *         5. 进入透传发送模式
  *         6. 构造并发送HTTP GET请求，包含城市代码和API Key
  *         7. 等待接收HTTP响应数据，超时3秒
  *         响应数据逐帧从u3缓冲区拷贝到response缓冲区
  *
  *         天气API使用固定Key（定义在WEATHER_KEY宏中）
  */
u8 Wifi_GetWeather(const char *city_code, char *response, u16 max_len)
{
    u32 timeout;
    u16 idx = 0;

    /* 第1步：发送"+++"退出透传模式 */
    usart3_send_str("+++");
    delay_ms(500);

    /* 第2步：关闭之前建立的TCP连接(如果有) */
    u3.flag = 0;
    u3.len = 0;
    usart3_send_str("AT+CIPCLOSE\r\n");
    delay_ms(500);

    /* 第3步：重新开启透传模式 */
    u3.flag = 0;
    u3.len = 0;
    if (Wifi_SendCmd("AT+CIPMODE=1\r\n", 2000))
    {
        return 1;
    }

    /* 第4步：建立TCP连接到天气API数据服务器 */
    u3.flag = 0;
    u3.len = 0;
    if (Wifi_SendCmd("AT+CIPSTART=\"TCP\",\"api.yytianqi.com\",80\r\n", 5000))
    {
        return 2;
    }

    /* 第5步：进入透传发送模式，准备发送数据 */
    u3.flag = 0;
    u3.len = 0;
    if (Wifi_SendCmd("AT+CIPSEND\r\n", 2000))
    {
        return 3;
    }

    /* 第6步：构造并发送HTTP GET请求获取天气数据 */
    u3.flag = 0;
    u3.len = 0;
    char req[128];
    sprintf(req, "GET /observe?city=%s&key=" WEATHER_KEY " HTTP/1.0\r\nHost: api.yytianqi.com\r\n\r\n", city_code);
    usart3_send_str(req);

    /* 第7步：等待并接收HTTP响应数据(超时3秒) */
    idx = 0;
    timeout = 0;
    while (timeout < 3000)       /* 最多等待3000ms */
    {
        if (u3.flag)             /* 检测到新的一帧数据到达 */
        {
            u3.flag = 0;
            for (u16 i = 0; i < u3.frame_len && idx < max_len - 1; i++)
            {
                response[idx++] = u3.buff[i];   /* 将数据拷贝到响应缓冲区 */
            }
            timeout = 0;         /* 收到数据后重置超时计数器(保证接收完整) */
        }
        delay_ms(1);
        timeout++;
    }
    response[idx] = '\0';        /* 字符串结束符 */

    return 0;
}
