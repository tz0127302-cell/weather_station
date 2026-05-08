/**
 * @file led.c
 * @brief LED指示灯驱动源文件
 * @details 实led的GPIO控制整体处理
 * @author He
 * @date 2026-04-25
 */

#include "led.h"


/* 
LED管脚初始化代码 
形参：无
返回参：无
函数说明：
    LED1  ---   PC5
    LED2  ---   PC4

*/

/**
 * @brief LED管脚初始化函数
 * @param void
 * @retval void
 * @details LED1(PC5)和LED2(PC4)初始化为通用推挽输出
 *         PC5和PC4输出高电平LED熄灭（低电平LED亮）
 */
void Led_Init(void)
{
    /* 步骤1：打开GPIOC时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    /* 步骤2：GPIO初始化结构体 */
    GPIO_InitTypeDef GPIO_InitStructure={0};

    /* 步骤3：配置GPIO管脚 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_4;      /* 选择PC5和PC4 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;             /* 通用推挽输出 */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;            /* 最大输出速率50MHz */
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* 步骤4：设置初始状态，输出高电平（LED熄灭）*/
    GPIO_SetBits(GPIOC, GPIO_Pin_5 | GPIO_Pin_4);
}






