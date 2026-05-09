/**
 * @file key_exti_part.c
 * @brief 按键EXTI中断初始化与服务函数源文件（独立模块）
 * @details 本文件单独提取按键的外部中断初始化与中断服务函数，
 *          用于实现PA0引脚的上升沿中断触发功能。
 *          与key.c中的同名函数功能相同，可作为独立模块使用
 * @author He
 * @date 2026-04-25
 */

/**
 * @brief 按键GPIO中断初始化 (PA0 - EXTI0)
 * @param void
 * @retval void
 * @details 配置PA0引脚为EXTI中断输入，上升沿触发，
 *          中断优先级设置为抢占优先级1、子优先级1
 * @note 调用此函数前需要先使能GPIOA时钟并初始化PA0引脚
 */
void Key_EXTI_Init(void)
{
    /* 使能AFIO（复用功能I/O）时钟，外部中断功能需要AFIO时钟支持 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    /* 将PA0引脚连接到EXTI线路0，建立GPIO引脚与EXTI外设的映射关系 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);

    /* 配置EXTI外设参数 */
    EXTI_InitTypeDef EXTI_InitStruct;
    EXTI_InitStruct.EXTI_Line = EXTI_Line0;              /* 选择外部中断线路0 */
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;     /* 设置为中断模式，产生中断信号给NVIC */
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;  /* 上升沿触发：按键按下瞬间电平由低变高 */
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;               /* 使能EXTI线路0 */
    EXTI_Init(&EXTI_InitStruct);

    /* 配置NVIC嵌套向量中断控制器，设定中断优先级 */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;            /* EXTI0中断通道号 */
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;   /* 抢占优先级为1（数值越小优先级越高） */
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;          /* 子优先级为1 */
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;             /* 使能EXTI0中断通道 */
    NVIC_Init(&NVIC_InitStruct);
}

/**
 * @brief EXTI0中断服务函数 (PA0按键触发)
 * @details 当PA0引脚检测到上升沿信号（按键被按下）时触发此中断。
 *          中断服务函数中设置全局标志位key_int_flag为1，
 *          然后清除中断挂起位，确保下次中断能够正常触发
 * @note 中断函数名EXTI0_IRQHandler由启动文件中的中断向量表定义，
 *       不可随意更改函数名
 */
void EXTI0_IRQHandler(void)
{
    /* 判断EXTI线路0是否确实触发了中断请求 */
    if (EXTI_GetITStatus(EXTI_Line0) == SET)
    {
        /* 将按键中断标志位置1，供主循环或任务处理函数查询 */
        key_int_flag = 1;
        /* 清除EXTI0的中断挂起标志位，防止退出中断后立即再次进入 */
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}
