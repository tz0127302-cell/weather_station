#include "app.h"
/*============================================================*
 * 文件名: app.c
 * 功能: FreeRTOS 应用层任务管理
 *          包含3个任务+1个定时器:
 *            - WeatherTask: 天气获取 (优先级2)
 *            - KeyTask:     按键扫描 + 语音识别 (优先级3)
 *            - DisplayTask: 串口打印 (优先级1)
 *            - Timer1:      DHT11 温湿度采集 (周期1000ms)
 *============================================================*/

/*======================== 共享变量 ============================
 * indoor_data:  由 Timer1 写入, DisplayTask 读取
 * weather_data: 由 WeatherTask 写入, 供 LCD/语音使用
 * current_city_idx: 当前城市索引
 *============================================================*/

/* 室内温湿度结构体, volatile 防止编译器优化 */
volatile struct _indoor_data indoor_data;

/* 当前城市的天气数据 (包含城市名, 天气状况, 温度, 湿度等) */
WeatherData weather_data;

/* 当前城市在 city_codes 中的索引 */
uint8_t current_city_idx = 0;

/* 城市代码表: 向天气API请求时使用 */
static const char *city_codes[] = {
    "CH281601",  /* 东莞 */
    "CH010100",  /* 北京 */
    "CH020100",  /* 上海 */
    "CH280101",  /* 广州 */
};
#define CITY_COUNT  (sizeof(city_codes) / sizeof(city_codes[0]))

/*======================== IPC 对象 ============================
 * weatherCmdQueue: 命令队列, KeyTask 发命令, WeatherTask 接收
 * Timer1Handle:    软件定时器句柄
 * TaskXxxHandle:   各任务句柄
 *============================================================*/

QueueHandle_t weatherCmdQueue;         /* 天气命令队列句柄 */
TimerHandle_t Timer1Handle;            /* 定时器1句柄 */
TaskHandle_t Task1Handle;              /* WeatherTask 句柄 */
TaskHandle_t Task2Handle;              /* KeyTask 句柄 */
TaskHandle_t Task3Handle;              /* DisplayTask 句柄 */
extern TaskHandle_t StartTaskHandle;   /* 启动任务句柄 (在 main.c) */

/* 队列命令码 */

/*============================================================*
 * 函数名: Timer1Callback
 * 功能: 定时器1回调函数, 每1000ms执行一次
 *       读取 DHT11 温湿度数据, 存入 indoor_data
 * 注意: 用 taskENTER_CRITICAL 保护 DHT11 时序,
 *       防止在读取过程中被任务切换打断导致数据错误
 *============================================================*/
void Timer1Callback(TimerHandle_t xTimer)
{
    uint8_t temp, humi;

    /* 进入临界区: 禁止任务切换, 保护DHT11微秒级时序 */
    taskENTER_CRITICAL();

    if (DHT11_Read_Data(&temp, &humi) == 0)
    {
        indoor_data.temp = temp;  /* 更新室内温度 */
        indoor_data.humi = humi;  /* 更新室内湿度 */
    }

    /* 退出临界区: 恢复任务切换 */
    taskEXIT_CRITICAL();
}

/*============================================================*
 * 函数名: WeatherTask
 * 功能: 天气获取任务, 优先级2
 *       1. 首次运行获取默认城市(东莞)天气
 *       2. 进入命令循环, 等待队列命令
 *       3. 收到 CMD_SWITCH_CITY 时切换并重新获取天气
 * 注意: buf[1024] 声明为 static,
 *       避免占用任务栈空间导致栈溢出
 *============================================================*/
void WeatherTask(void *ptr)
{
    uint8_t cmd;           /* 从队列接收的命令码 */
    static char buf[1024]; /* 天气API响应缓冲区(静态区, 不在栈上) */

    /* ---- 首次启动: 获取第0个城市(东莞)的天气 ---- */
    current_city_idx = 0;
    if (Wifi_GetWeather(city_codes[0], buf, sizeof(buf)) == 0)
    {
        Weather_Parse(buf, &weather_data);
        printf("city: %s, weather: %s, temp: %d C, hum: %d%%\r\n",
               weather_data.cityName, weather_data.tq,
               weather_data.qw, weather_data.sd);
        Weather_Display(&weather_data);
        /* 首次不播报语音, 加快启动速度 */
    }

    /* ---- 命令循环 ---- */
    while (1)
    {
        /* 阻塞等待队列命令, portMAX_DELAY = 无限等待 */
        if (xQueueReceive(weatherCmdQueue, &cmd, portMAX_DELAY) == pdPASS)
        {
            if (cmd == CMD_SWITCH_CITY)
            {
                Lcd_DisplayPic(0,0,gImage_desktop); /* 显示桌面背景图 */
                /* 切换到下一个城市 */
                current_city_idx = (current_city_idx + 1) % CITY_COUNT;
                printf("switch to city %d\r\n", current_city_idx);

                if (Wifi_GetWeather(city_codes[current_city_idx], buf, sizeof(buf)) == 0)
                {
                    printf("get weather\r\n");
                    Weather_Parse(buf, &weather_data);
                    printf("city: %s, weather: %s, temp: %d C, hum: %d%%\r\n",
                           weather_data.cityName, weather_data.tq,
                           weather_data.qw, weather_data.sd);

                    Weather_Display(&weather_data);   /* 更新LCD显示 */
                    Weather_Voice_Play(weather_data.voice_file,
                                       weather_data.qw, weather_data.sd,10+current_city_idx);  /* 语音播报 */
                }
                else
                {
                    printf("weather fetch failed for city %d\r\n", current_city_idx);
                }
            }
            else if (cmd == CMD_WEATHER_VOICE)
            {
                /* voice command: broadcast cached weather via WeatherTask */
                Weather_Voice_Play(weather_data.voice_file,
                                   weather_data.qw, weather_data.sd,
                                   10 + current_city_idx);
            }
        }
    }
}

/*============================================================*
 * 函数名: KeyTask
 * 功能: 按键扫描任务, 优先级3(最高)
 *       1. 轮询 Key_Scan() 检测按键
 *       2. 按键按下时发送 CMD_SWITCH_CITY 到命令队列
 *       3. 轮询 SU03T 语音识别模块
 *       4. vTaskDelay(20) 让出CPU, 避免长时间100%占用
 *============================================================*/
void KeyTask(void *ptr)
{
    uint8_t key;  /* 按键扫描返回值, 1=按下并释放 */

    while (1)
    {
        key = Key_Scan();
        if (key == 1)
        {
            uint8_t cmd = CMD_SWITCH_CITY;
            xQueueSend(weatherCmdQueue, &cmd, 0);  /* 发送到队列, 不等待 */
        }

        /* 轮询语音识别模块 */
        SU_03T_Control();

        /* 延时20ms, 让低优先级任务有执行机会 */
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

/*============================================================*
 * 函数名: DisplayTask
 * 功能: 显示任务, 优先级1(最低)
 *       每2秒在串口打印室内温湿度和RTC时间
 *       后续可扩展为LCD同时显示室内外数据
 *============================================================*/
void DisplayTask(void *ptr)
{
    while (1)
    {
        char line[32];
        /* 串口打印室内温湿度 */
        Lcd_DisplayPic(0,0,gImage_desktop); /* 显示桌面背景图 */
        Weather_Display(&weather_data);   /* 更新LCD显示 */
        printf("Indoor: %dC, %d%%\r\n", indoor_data.temp, indoor_data.humi);
        /*屏幕显示室内温度*/
        Lcd_DisplayStr(160, 80, BLACK, WHITE, 24, (u8 *)"室温:");
        sprintf(line, "%d℃", indoor_data.temp);
        Lcd_DisplayStr(240, 80, BLACK, WHITE, 24, (u8 *)line);
        /*屏幕显示室内湿度*/
        Lcd_DisplayStr(160, 120, BLACK, WHITE, 24, (u8 *)"室湿:");
        sprintf(line, "%d%%", indoor_data.humi);
        Lcd_DisplayStr(240, 120, BLACK, WHITE, 24, (u8 *)line);
        
        /* 串口打印RTC日期时间 */
        RTC_Analysis();
        /* 屏幕显示RTC日期时间 */
        sprintf(line, "%04d-%02d-%02d", rtc_time.year, rtc_time.month, rtc_time.day);
        Lcd_DisplayStr(10, 10, BLACK, WHITE, 24, (u8 *)line);

        sprintf(line, "%02d:%02d:%02d", rtc_time.hour, rtc_time.minute, rtc_time.second);
        Lcd_DisplayStr(80, 40, BLACK, WHITE, 32, (u8 *)line);


        /* 延时3秒 ,等工程调试好后改为60s更新一次*/
        vTaskDelay(60000 / portTICK_PERIOD_MS);
    }
}

/*============================================================*
 * 函数名: StartTask
 * 功能: 系统启动任务, 优先级4(最高)
 *       由 main() 创建, 负责初始化所有应用资源:
 *       创建3个任务 + 1个队列 + 1个定时器
 *       完成后删除自身以回收栈空间
 *============================================================*/
void StartTask(void *ptr)
{
    printf("StartTask is running\r\n");

    /* ---- 创建3个应用任务 ---- */
    xTaskCreate(WeatherTask,            /* 任务函数入口 */
                "WeatherTask",           /* 任务名称(调试用) */
                START_TASK_STACK_SIZE,   /* 栈大小(单位: 字) */
                NULL,                    /* 任务参数 */
                2,                       /* 优先级 */
                &Task1Handle);           /* 任务句柄 */

    xTaskCreate(KeyTask,
                "KeyTask",
                START_TASK_STACK_SIZE,
                NULL,
                3,                       /* 最高优先级, 保证按键响应 */
                &Task2Handle);

    xTaskCreate(DisplayTask,
                "DisplayTask",
                START_TASK_STACK_SIZE,
                NULL,
                1,                       /* 最低优先级 */
                &Task3Handle);

    /* ---- 创建命令队列 (深度10, 每项1字节) ---- */
    weatherCmdQueue = xQueueCreate(10, sizeof(uint8_t));

    /* ---- 创建并启动软件定时器1 (DHT11, 周期1000ms) ---- */
    Timer1Handle = xTimerCreate("Timer1",
                                1000 / portTICK_PERIOD_MS,  /* 1000tick = 1秒 */
                                pdTRUE,                     /* 自动重载 */
                                NULL,
                                Timer1Callback);

    if (Timer1Handle != NULL)
    {
        xTimerStart(Timer1Handle, 0);  /* 立即启动定时器 */
    }

    /* 删除启动任务, 释放栈空间 */
    vTaskDelete(StartTaskHandle);
}
