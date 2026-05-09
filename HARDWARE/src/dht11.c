/**
 * @file dht11.c
 * @brief DHT11温湿度传感器驱动程序源文件
 * @details 实现DHT11传感器的单总线通信协议，包含初始化、复位、
 *         响应检测、位读取、字节读取以及完整温湿度数据读取功能。
 *         通信时序要求严格，延时需使用微秒级精确延时函数。
 * @author He
 * @date 2026-04-25
 */

#include "dht11.h"

/**
 * @brief 配置 DHT11 引脚为推挽输出模式
 * @param void
 * @retval void
 * @details 将PC1引脚配置为推挽输出，用于主机向DHT11发送
 *         起始信号和复位信号。输出速度为50MHz。
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
 * @brief 配置 DHT11 引脚为浮空输入模式
 * @param void
 * @retval void
 * @details 将PC1引脚配置为浮空输入，用于接收DHT11返回的
 *         响应信号和数据位。浮空输入模式下引脚电平由外部
 *         设备（DHT11）驱动。
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
 * @param void
 * @retval void
 * @details 主机发送起始信号：先将数据线拉低至少18ms（此处延时20ms），
 *         然后再拉高20~40us（此处延时30us），等待DHT11响应。
 *         这是DHT11单总线通信的第一步，必须在每次读取数据前调用。
 * @note 时间关键：拉低必须 >= 18ms（DHT11的识别窗口），
 *       拉高必须 20~40us（DHT11将在之后拉低总线作为响应）
 */
void DHT11_Rst(void)
{
    DHT11_Mode_Out();       /* 切换为输出模式，主机准备发送起始信号 */
    DHT11_DQ_OUT_L;         /* 主机拉低数据线 */
    delay_ms(20);           /* 拉低至少 18ms（保证DHT11能检测到起始信号） */
    DHT11_DQ_OUT_H;         /* 主机拉高数据线 */
    delay_us(30);           /* 主机拉高 20~40us（等待DHT11响应） */
}

/**
 * @brief 检测 DHT11 响应
 * @return 0:检测到DHT11响应（成功）, 1:未检测到响应（失败）
 * @details DHT11在收到起始信号后会拉低总线80us作为响应，
 *         然后再拉高80us准备发送数据。本函数检测这个响应过程：
 *         先等待DHT11拉低（响应标志），再等待DHT11拉高。
 * @note 每次超时等待约100us，超时则判定为无响应。
 */
uint8_t DHT11_Check(void)
{
    uint8_t retry = 0;
    DHT11_Mode_In();        /* 切换为输入模式，等待DHT11拉低总线 */

    /* 等待 DHT11 拉低总线（DHT11以拉低80us作为响应信号） */
    while (DHT11_DQ_IN && retry < 100) {
        retry++;
        delay_us(1);
    }
    if (retry >= 100) return 1;     /* 超时，无响应 */

    /* 等待 DHT11 拉高总线（拉低80us后释放总线为下一次通信做准备） */
    retry = 0;
    while (!DHT11_DQ_IN && retry < 100) {
        retry++;
        delay_us(1);
    }
    if (retry >= 100) return 1;     /* 超时，响应异常 */
    return 0;                       /* 响应正常 */
}

/**
 * @brief 从 DHT11 读取一个位 (Bit)
 * @return 读取到的数据位，返回 1 或 0
 * @details DHT11数据位读取时序：
 *         1. 主机等待DHT11拉低总线（数据起始标志）
 *         2. 主机等待DHT11拉高总线（准备发送数据）
 *         3. 延时40us后采样电平：
 *            - 如果为高电平且持续26~28us，表示数据位"1"
 *            - 如果为低电平，表示数据位"0"
 *         这里采用延时40us后直接检测电平的方式判断数据位。
 * @note 时序要求严格，延时40us是关键的采样点。
 *       50us以内的短低电平表示"0"，50us以上的高低电平表示"1"。
 */
uint8_t DHT11_Read_Bit(void)
{
    uint8_t retry = 0;

    /* 等待DHT11拉低总线（数据位起始标志，每位的开始DHT11都会拉低总线50us） */
    while (DHT11_DQ_IN && retry < 100) {
        retry++;
        delay_us(1);
    }

    /* 等待DHT11释放总线（拉高），准备发送实际数据 */
    retry = 0;
    while (!DHT11_DQ_IN && retry < 100) {
        retry++;
        delay_us(1);
    }

    delay_us(40);           /* 延时 40us 后检测电平（在数据位中间位置采样） */

    /* 如果40us后仍为高电平，表示数据位为"1"；否则为"0" */
    if (DHT11_DQ_IN) return 1;
    else return 0;
}

/**
 * @brief 从 DHT11 读取一个字节 (Byte)
 * @return 读取到的8位字节数据
 * @details 连续调用8次DHT11_Read_Bit()，将读取到的位按
 *          MSB（最高位）在先的方式组合成一个字节。
 *         数据格式：高位在前，低位在后。
 */
uint8_t DHT11_Read_Byte(void)
{
    uint8_t i, dat = 0;

    for (i = 0; i < 8; i++) {
        dat <<= 1;              /* 左移一位，为新位留出空间 */
        dat |= DHT11_Read_Bit();/* 读取一个位并拼接到最低位 */
    }
    return dat;
}

/**
 * @brief 初始化 DHT11 传感器
 * @return 0:初始化成功, 1:初始化失败（传感器未响应）
 * @details 使能GPIOC时钟，发送复位信号，检测DHT11是否在线。
 *         必须在首次读取数据前调用。
 */
uint8_t DHT11_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);  /* 使能GPIOC时钟 */

    DHT11_Rst();                /* 发送复位信号 */
    return DHT11_Check();       /* 检测响应，返回0表示成功 */
}

/**
 * @brief 读取温湿度数据
 * @param temp 温度存储变量指针（输出参数）
 * @param humi 湿度存储变量指针（输出参数）
 * @return 0:读取成功, 1:读取失败
 * @details DHT11一次完整数据读取流程：
 *         1. 发送复位信号（拉低>18ms，拉高20~40us）
 *         2. 检测DHT11响应
 *         3. 如果响应成功，连续读取5个字节（40位数据）：
 *            byte[0] = 湿度整数部分
 *            byte[1] = 湿度小数部分（本驱动未使用）
 *            byte[2] = 温度整数部分
 *            byte[3] = 温度小数部分（本驱动未使用）
 *            byte[4] = 校验和（byte[0]+byte[1]+byte[2]+byte[3]）
 *         4. 校验数据完整性：前4字节之和 == 第5字节
 * @note 两次读取间隔建议大于1秒，DHT11的采样周期为1秒。
 *       小数部分在此驱动中被忽略，如需更高精度可扩展读取。
 */
uint8_t DHT11_Read_Data(uint8_t *temp, uint8_t *humi)
{
    uint8_t buf[5];             /* 数据缓冲区：5字节 */
    uint8_t i;

    DHT11_Rst();                /* 发送复位信号，启动一次通信 */

    if (DHT11_Check() == 0) {   /* 检测到DHT11响应 */
        for (i = 0; i < 5; i++) {
            buf[i] = DHT11_Read_Byte();  /* 依次读取5个字节 */
        }

        /* 校验数据：前4个字节（湿度整数+湿度小数+温度整数+温度小数）之和等于校验和 */
        if ((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4]) {
            *humi = buf[0];     /* 湿度整数部分 */
            *temp = buf[2];     /* 温度整数部分 */
            return 0;           /* 读取成功 */
        }
    }
    return 1;                   /* 读取失败（无响应或校验错误） */
}
