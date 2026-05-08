/**
 * @file main.c
 * @brief
 * @details
 * @author He
 * @date 2026-04-25
 */

#include "main.h"
#include "pic.h"

TaskHandle_t StartTaskHandle;


/* city codes for cycling */
static const char *city_codes[] = {
    "CH010100",  /* BeiJing */
    "CH020100",  /* ShangHai */
    "CH280101",  /* GuangZhou */
    "CH281601",  /* ShenZhen */
};
#define CITY_COUNT  (sizeof(city_codes) / sizeof(city_codes[0]))


int main()
{
    //
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    Led_Init();//LED
    USART1_Init(115200);  // 115200
    LCD_Init();//LCD
    DHT11_Init();//DHT11
    W25Qxx_Config();//W25QXX
    SU_03T_Config();//SU03T
    MY1680_Config();//MY1680
    Wifi_NtpInit();//WIFINTP
    Rtc_Init();//RTC
    Key_EXTI_Init();//KEY EXTI

    /* vars */
    u8 city_idx = 0;
    u8 busy = 0;
    u8 loop_cnt = 0;
    char buf[1024];
    WeatherData w;

    // if(KEY1)
    // {
    //   Font_Update();
    // }

    xTaskCreate(StartTask,    // 任务函数指针
                "StartTask",    // 任务名称
                START_TASK_STACK_SIZE,    // 任务栈大小
                "hello start",  // 任务参数
                START_TASK_PRIORITY,  // 任务优先级
                &StartTaskHandle );  // 任务句柄    
    vTaskStartScheduler(); // 启动调度器
    /* fetch default city weather */
    if (Wifi_GetWeather(city_codes[city_idx], buf, sizeof(buf)) == 0)
    {
        Weather_Parse(buf, &w);
        printf("city: %s, weather: %s, temp: %d C, hum: %d%%\r\n",
               w.cityName, w.tq, w.qw, w.sd);

        /* display on LCD */
        Weather_Display(&w);

        /* voice broadcast */
        Weather_Voice_Play(w.voice_file, w.qw, w.sd);
    }
    else
    {
        printf("weather fetch failed\r\n");
    }

    /* main loop */
    while(1)
    {
        printf("进入主循环\r\n");
        /* periodic tasks (every ~1s) */
        loop_cnt++;
        if (loop_cnt >= 10)
        {
            loop_cnt = 0;
            SU_03T_Control();
            RTC_Analysis();
        }

        /* button switch city (interrupt trigger) */
        if (!busy && key_int_flag)
        {
            key_int_flag = 0;
            delay_ms(20);  /* debounce */
            if (KEY1 != Bit_SET) continue;  /* false trigger */

            busy = 1;
            city_idx = (city_idx + 1) % CITY_COUNT;
            printf("switch to city %d\r\n", city_idx);

            if (Wifi_GetWeather(city_codes[city_idx], buf, sizeof(buf)) == 0)
            {
                Weather_Parse(buf, &w);
                printf("city: %s, weather: %s, temp: %d C, hum: %d%%\r\n",
                       w.cityName, w.tq, w.qw, w.sd);

                /* refresh LCD */
                Weather_Display(&w);

                /* voice broadcast */
                Weather_Voice_Play(w.voice_file, w.qw, w.sd);
            }
            else
            {
                printf("weather fetch failed for city %d\r\n", city_idx);
            }
            busy = 0;
        }
        Lcd_DisplayPic(160,10,gImage_00_0);
        delay_ms(100);
    }
}
