/**
 * @file led.c
 * @brief LED指示灯驱动程序源文件
 * @details 实现LED的GPIO初始化及亮灭控制
 * @author He
 * @date 2026-04-25
 */

#include "led.h"


/*
LED引脚初始化函数
参数：无
返回值：无
引脚说明：
    LED1  ---   PC5
    LED2  ---   PC4

*/

/**
 * @brief LED引脚初始化函数
 * @param void
 * @retval void
 * @details LED1(PC5)和LED2(PC4)初始化为推挽输出模式
 *         PC5和PC4输出高电平LED熄灭（低电平LED点亮）
 */
void Led_Init(void)
{
    /* 步骤1：使能GPIOC时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    /* 步骤2：定义GPIO初始化结构体 */
    GPIO_InitTypeDef GPIO_InitStructure={0};

    /* 步骤3：配置GPIO引脚 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_4;      /* 选择PC5和PC4 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;             /* 推挽输出模式 */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;            /* 输出频率最高50MHz */
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* 步骤4：设置初始状态，输出高电平使LED熄灭（低电平点亮）*/
    GPIO_SetBits(GPIOC, GPIO_Pin_5 | GPIO_Pin_4);
}







