/**
 * @file main.c
 * @brief 系统主函数入口
 * @details 硬件初始化 -> 创建启动任务 -> 启动FreeRTOS调度器
 *          所有业务逻辑由 FreeRTOS 任务管理，main() 只负责初始化
 * @author He
 * @date 2026-04-25
 */

#include "main.h"

TaskHandle_t StartTaskHandle;  /* 启动任务句柄，供 app.c 使用 */

/**
 * @brief 系统主函数
 *        1. 初始化所有硬件外设
 *        2. 创建 StartTask 启动任务
 *        3. 启动 FreeRTOS 调度器（永不返回）
 * @param 无
 * @retval int 正常情况下不返回
 */
int main()
{
    /*==================== 第1步：硬件初始化 ====================*/

    /* 设置中断优先级分组为2位抢占 + 2位响应 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    Led_Init();            /* 初始化 LED: LED1(PC5), LED2(PC4) */
    USART1_Init(115200);   /* 初始化串口1: 控制台输出, 波特率115200 */
    LCD_Init();            /* 初始化 LCD: ST7789, 320x240 */
    DHT11_Init();          /* 初始化 DHT11: 温湿度传感器, PC1 */
    W25Qxx_Config();       /* 初始化 W25Q64: SPI Flash 存储 */
    SU_03T_Config();       /* 初始化 SU03T: 语音识别模块, UART4 */
    MY1680_Config();       /* 初始化 MY1680: 语音合成模块, UART5 */
    Lcd_DisplayPic(0,0,gImage_desktop); /* 显示桌面背景图 */
    Wifi_NtpInit();        /* 初始化 WiFi + NTP: ESP-12F, 连接AP并同步时间 */
    Rtc_Init();            /* 初始化 RTC: 内部实时时钟, 1Hz */
    Key_Init();            /* 初始化按键: PA0, 上拉输入 */

    /*==================== 第2步：创建启动任务 ====================*/

    xTaskCreate(StartTask,            /* 启动任务函数 */
                "StartTask",           /* 任务名(调试用) */
                START_TASK_STACK_SIZE, /* 栈大小(256 words = 1024 bytes) */
                "hello start",         /* 任务参数 */
                START_TASK_PRIORITY,   /* 优先级4(最高) */
                &StartTaskHandle);     /* 任务句柄 */

    /*==================== 第3步：启动调度器 ====================*/

    vTaskStartScheduler();  /* 启动 FreeRTOS 调度器 */

    /* 正常情况下 vTaskStartScheduler() 永不返回 */
    /* 如果执行到这里说明调度器启动失败(通常是堆内存不足) */
    while(1);  /* 死循环，防止程序跑飞 */
}
