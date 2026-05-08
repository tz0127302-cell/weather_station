/**
 * @file wifi.c
 * @brief WIFI模块驱动程序源文件
 * @details WIFI用于控制和NTP时间同步
 * @author He
 * @date 2026-04-25
 */

#include "wifi.h"

u32 timestamp = 0;
u8 ntp_flag = 0;
/**
  * @brief USART3初始化
  * @param void
  * @retval void
  * @author He
  *
    USART3_TX    ---    PB10
    USART3_RX    ---    PB11
    WIFI_Enable  ---    PC9       ---   模块使能脚
  */

void Usart3_Config(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC,ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);


    GPIO_InitTypeDef GPIO_InitStructure={0};
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIOC->ODR |= 1 << 9;//使能WIFI模块

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);


    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);



    //USART3初始化
    USART_InitTypeDef USART_InitStruct={0};
    USART_InitStruct.USART_BaudRate = 115200;//波特率
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//RTS  CTS
    USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART3,&USART_InitStruct);



    //中断使能
    USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);//使能接收中断
    USART_ITConfig(USART3,USART_IT_IDLE,ENABLE);//使能空闲中断

    //NVIC
    NVIC_SetPriority(USART3_IRQn,NVIC_EncodePriority(5, 1, 1));
    NVIC_EnableIRQ(USART3_IRQn);

    //使能USART3
    USART_Cmd(USART3,ENABLE);

}

/**
  * @brief USART3中断处理函数
  * @param  void
  * @retval void
  * @author He
  *
  */
U3 u3;
void USART3_IRQHandler(void)
{

    if(USART_GetITStatus(USART3,USART_IT_RXNE))
    {
        USART_ClearITPendingBit(USART3,USART_IT_RXNE);
        //user handle

        if (u3.len < sizeof(u3.buff))
            u3.buff[u3.len++] = USART_ReceiveData(USART3);

    }
    if(USART_GetITStatus(USART3,USART_IT_IDLE))
    {
        USART3->SR;
        USART3->DR;
        //user handle

        u3.buff[u3.len] = '\0';
        u3.frame_len = u3.len;
        u3.len = 0;
        u3.flag = 1;//表示接收数据完成


    }


}


/**
  * @brief USART3发送1byte数据
  * @param u8 data
  * @retval void
  * @author He
  *
    查询发送寄存器为空标志位为1，查询数据寄存器标志位是否1，
    在发送之前
  */

void usart3_send_byte(u8 data)
{

    while(!USART_GetFlagStatus(USART3,USART_FLAG_TXE));
    USART_SendData(USART3, data);

}




/**
  * @brief USART3发送字符串（发送多字节的数据）
  * @param u8 *str   "1234567\0"
  * @retval void
  * @author He
  *
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
  * @brief ESP-12F 发送AT指令，判断OK
  * @param
            char *cmd    需要发送给ESP12F的AT指令
            u32 time_out 设定的等待超时时间

  * @retval u8    0:成功      1:失败
  * @author He
  *
  */

u8 Wifi_SendCmd(char *cmd,u32 time_out)
{
    u32 count = 0;
    //发送AT指令
    usart3_send_str(cmd);
    delay_ms(300);

    //等待ESP-12F回复，等待OK
    while(1)
    {
        //等待数据接收完成
        while(!u3.flag)
        {
          count++;
          delay_ms(1);
          if(count >= time_out)
          {
              u3.len = 0;
              return 1;
          }

        }
        //U3成功收到ESP12F回复数据
        u3.flag = 0;
        //判断u3.buff数据中是否包含"OK"
        if(strstr((const char *)u3.buff,"OK") != NULL)
        {
            return 0;
        }

    }


}






/**
  * @brief ESP-12F WIFI模块TCP初始化
  * @param  void
  * @retval u8    0:网络连接成功    非零：网络连接失败
  * @author He
  *
  */

u8 Wifi_NtpInit(void)
{
    u8 ret = 0;

    //初始化USART2
    Usart3_Config();

    //发送"+++"退出透传模式
    usart3_send_str("+++");
    delay_ms(300);
    //发送"AT\r\n"测试模块
    usart3_send_str("AT\r\n");
    delay_ms(300);
    //发送恢复出厂设置指令"AT+RESTORE\r\n"
    usart3_send_str("AT+RESTORE\r\n");
    delay_ms(1000);
    //设置WIFI模式为客户端模式
    ret = Wifi_SendCmd("AT+CWMODE=1\r\n",2000);
    if(ret)
    {
        printf("客户端模式失败\r\n");
        return 1;
    }
    printf("客户端模式成功\r\n");
    //连接AP
    ret = Wifi_SendCmd("AT+CWJAP=\"WIFI-6G\",\"11111111\"\r\n",15000);
    if(ret)
    {
        printf("连接AP失败\r\n");
        return 2;
    }
    printf("连接AP成功\r\n");
    //设置透传模式
    ret = Wifi_SendCmd("AT+CIPMODE=1\r\n",2000);
    if(ret)
    {
        printf("设置透传模式失败\r\n");
        return 3;
    }
    printf("设置透传模式成功\r\n");
    //连接到NTP服务器
    ret = Wifi_SendCmd("AT+CIPSTART=\"TCP\",\"api.k780.com\",80\r\n",2000);
    if(ret)
    {
        printf("连接NTP服务器失败\r\n");
        return 4;
    }
    printf("连接NTP服务器成功\r\n");
    //进入发送模式
     ret = Wifi_SendCmd("AT+CIPSEND\r\n",2000);
    if(ret)
    {
        printf("进入发送模式失败\r\n");
        return 5;
    }
    printf("进入发送模式成功\r\n");
    //发送NTP请求
    usart3_send_str("GET http://api.k780.com:88/?app=life.time&appkey=10003&sign=b59bc3ef6191eb9f747dd4e83c99f2a4&format=json&HTTP/1.1\r\n");

    delay_ms(1000);

    sscanf(&u3.buff[20],"%*[^0123456789]%d",&timestamp);
    printf("timestamp:%d\r\n",timestamp);

    ntp_flag = 1;


    return ret;

}


/**
  * @brief  WIFI模块天气数据获取
  * @param  response: 存放HTTP响应的缓冲区
  * @param  max_len: 缓冲区大小
  * @retval u8  0:成功  1:TCP连接失败  2:CIPSEND超时  3:响应超时
  * @author He
  * @date 2026-05-06
  */
u8 Wifi_GetWeather(const char *city_code, char *response, u16 max_len)
{
    u32 timeout;
    u16 idx = 0;

    /* 第1步：先退出透传模式 */
    usart3_send_str("+++");
    delay_ms(500);

    /* 第2步：关闭上一个TCP连接 */
    u3.flag = 0;
    u3.len = 0;
    usart3_send_str("AT+CIPCLOSE\r\n");
    delay_ms(500);

    /* 第3步：设置透传模式 */
    u3.flag = 0;
    u3.len = 0;
    if (Wifi_SendCmd("AT+CIPMODE=1\r\n", 2000))
    {
        return 1;
    }

    /* 第4步：连接天气API服务器 */
    u3.flag = 0;
    u3.len = 0;
    if (Wifi_SendCmd("AT+CIPSTART=\"TCP\",\"api.yytianqi.com\",80\r\n", 5000))
    {
        return 2;
    }

    /* 第5步：进入透传发送模式 */
    u3.flag = 0;
    u3.len = 0;
    if (Wifi_SendCmd("AT+CIPSEND\r\n", 2000))
    {
        return 3;
    }

    /* 第6步：发送HTTP GET请求 */
    u3.flag = 0;
    u3.len = 0;
    char req[128];
    sprintf(req, "GET /observe?city=%s&key=" WEATHER_KEY " HTTP/1.0\r\nHost: api.yytianqi.com\r\n\r\n", city_code);
    usart3_send_str(req);

    /* 第7步：接收HTTP响应 */
    idx = 0;
    timeout = 0;
    while (timeout < 3000)
    {
        if (u3.flag)
        {
            u3.flag = 0;
            for (u16 i = 0; i < u3.frame_len && idx < max_len - 1; i++)
            {
                response[idx++] = u3.buff[i];
            }
            timeout = 0;  /* 收到数据则重置超时 */
        }
        delay_ms(1);
        timeout++;
    }
    response[idx] = '\0';

    return 0;
}
