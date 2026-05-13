/**
 * @file key.c
 * @brief 按键检测函数源文件
 * @details 包含按键GPIO初始化、按键扫描函数以及按键EXTI中断初始化与中断服务函数
 *          PA0引脚作为按键输入，上升沿触发中断
 * @author He
 * @date 2026-04-25
 */

#include "key.h"




/**
 * @brief 按键引脚初始化
 * @param void
 * @retval void
 * @details 将PA0配置为浮空输入模式，用于检测外部按键电平变化
 *          浮空输入模式下，引脚电平由外部电路决定，内部无上下拉
 */
void Key_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* 配置PA0为浮空输入模式 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/**
 * @brief 按键扫描函数（阻塞式）
 * @param void
 * @retval uint8_t 1=检测到按键按下并释放，0=无按键动作
 * @details 检测PA0引脚是否为高电平，若是则延时30ms去抖，
 *          然后等待按键释放（电平变低），返回1表示一次完整的按键操作
 * @note 此函数为阻塞式，会等待按键释放后才返回
 */
uint8_t Key_Scan(void)
{
    /* 检测按键是否按下（PA0是否为高电平） */
    if(KEY1 == Bit_SET)
    {
        /* 延时30ms进行软件去抖，消除按键机械抖动造成的影响 */
        delay_ms(30);
        /* 等待按键释放，阻塞在此直到PA0变为低电平 */
        while(KEY1 == Bit_SET);
        /* 返回1，表示检测到一次完整的按键按下并释放操作 */
        return 1;
    }
    /* 无按键按下，返回0 */
    return 0;
}

