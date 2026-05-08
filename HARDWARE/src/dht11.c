#include "dht11.h"

/**
 * @brief 配置 DHT11 引脚为输出模式
 */
void DHT11_Mode_Out(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

/**
 * @brief 配置 DHT11 引脚为输入模式
 */
void DHT11_Mode_In(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

/**
 * @brief 复位 DHT11 传感器
 */
void DHT11_Rst(void)
{
    DHT11_Mode_Out();
    DHT11_DQ_OUT_L;
    delay_ms(20); // 拉低至少 18ms
    DHT11_DQ_OUT_H;
    delay_us(30); // 主机拉高 20~40us
}

/**
 * @brief 检测 DHT11 响应
 * @return 0:成功, 1:失败
 */
uint8_t DHT11_Check(void)
{
    uint8_t retry = 0;
    DHT11_Mode_In();
    
    // 等待 DHT11 拉低
    while (DHT11_DQ_IN && retry < 100) {
        retry++;
        delay_us(1);
    }
    if (retry >= 100) return 1;
    
    // 等待 DHT11 拉高
    retry = 0;
    while (!DHT11_DQ_IN && retry < 100) {
        retry++;
        delay_us(1);
    }
    if (retry >= 100) return 1;
    return 0;
}

/**
 * @brief 从 DHT11 读取一个位 (Bit)
 * @return 1 或 0
 */
uint8_t DHT11_Read_Bit(void)
{
    uint8_t retry = 0;
    
    while (DHT11_DQ_IN && retry < 100) {
        retry++;
        delay_us(1);
    }
    
    retry = 0;
    while (!DHT11_DQ_IN && retry < 100) {
        retry++;
        delay_us(1);
    }
    
    delay_us(40); // 延时 40us 后检测电平
    
    if (DHT11_DQ_IN) return 1;
    else return 0;
}

/**
 * @brief 从 DHT11 读取一个字节 (Byte)
 * @return 读取到的字节数据
 */
uint8_t DHT11_Read_Byte(void)
{
    uint8_t i, dat = 0;
    
    for (i = 0; i < 8; i++) {
        dat <<= 1;
        dat |= DHT11_Read_Bit();
    }
    return dat;
}

/**
 * @brief 初始化 DHT11
 * @return 0:成功, 1:失败
 */
uint8_t DHT11_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    
    DHT11_Rst();
    return DHT11_Check();
}

/**
 * @brief 读取温湿度数据
 * @param temp 温度存储变量指针
 * @param humi 湿度存储变量指针
 * @return 0:成功, 1:失败
 */
uint8_t DHT11_Read_Data(uint8_t *temp, uint8_t *humi)
{
    uint8_t buf[5];
    uint8_t i;
    
    DHT11_Rst();
    
    if (DHT11_Check() == 0) {
        for (i = 0; i < 5; i++) {
            buf[i] = DHT11_Read_Byte();
        }
        
        // 校验数据
        if ((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4]) {
            *humi = buf[0];  // 湿度整数部分
            *temp = buf[2];  // 温度整数部分
            return 0;
        }
    }
    return 1;
}
