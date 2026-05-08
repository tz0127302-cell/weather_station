/**
 * @file key.c
 * @brief 按键检测函数源文件
 * @details 包含按键GPIO初始化、扫描函数、边沿检测和EXTI中断
 * @author He
 * @date 2026-04-25
 */

#include "key.h"

/* 按键中断标志位 */
volatile uint8_t key_int_flag = 0;


/**
 * @brief 按键引脚初始化
 * @param void
 * @retval void
 */
void Key_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/**
 * @brief 按键扫描 (阻塞)
 * @param void
 * @retval uint8_t 1=按下并释放
 */
uint8_t Key_Scan(void)
{
    if(KEY1 == Bit_SET)
    {
        delay_ms(30);
        while(KEY1 == Bit_SET);
        return 1;
    }
    return 0;
}


/**
 * @brief 按键EXTI中断初始化 (PA0 - EXTI0)
 * @param void
 * @retval void
 */
void Key_EXTI_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);

    EXTI_InitTypeDef EXTI_InitStruct;
    EXTI_InitStruct.EXTI_Line = EXTI_Line0;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/**
 * @brief EXTI0中断服务函数 (PA0按键)
 */
void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) == SET)
    {
        key_int_flag = 1;
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}
