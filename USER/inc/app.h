#ifndef _APP_H
#define _APP_H

#include "main.h"

/*
 * FreeRTOS任务配置宏定义
 * START_TASK_STACK_SIZE: 启动任务的栈空间大小，单位为字，
 *                        用于创建其他任务时的栈大小，设为256字足够
 * START_TASK_PRIORITY:  启动任务优先级，数值越大优先级越高，
 *                        此处设为4，为最高优先级
 */
#define START_TASK_STACK_SIZE 256
#define START_TASK_PRIORITY  4

/* ======================== IPC 命令码 ========================
 * 通过 weatherCmdQueue 传递的命令定义
 * CMD_SWITCH_CITY:   切换城市，重新获取天气数据并播报
 * CMD_WEATHER_VOICE: 播报当前缓存的天气数据，不重新请求API
 *==================================================================*/
#define CMD_SWITCH_CITY     1
#define CMD_WEATHER_VOICE   2

/* ======================== 室内温湿度数据 ========================
 * 由 Timer1定时器中断定期读取 DHT11 并更新，
 * DisplayTask 和 Voice 功能模块通过此变量获取当前室内温湿度
 * temp: 温度，单位摄氏度
 * humi: 湿度，单位百分比
 *==================================================================*/
extern volatile struct _indoor_data
{
    uint8_t temp;
    uint8_t humi;
} indoor_data;

/* 天气任务的消息队列句柄，用于 KeyTask 向 WeatherTask 发送命令 */
extern QueueHandle_t weatherCmdQueue;

/* ======================== 任务函数声明 ========================
 * StartTask:      系统启动任务，创建 WeatherTask/KeyTask/DisplayTask 和定时器
 * WeatherTask:    天气数据采集任务，负责获取网络天气数据并播报
 * KeyTask:        按键扫描任务，负责检测硬件按键和 SU-03T 语音命令
 * DisplayTask:    显示刷新任务，负责更新 LCD 屏幕内容
 * Timer1Callback: 定时器回调，每隔1000ms读取 DHT11 并更新 indoor_data
 *==================================================================*/
void StartTask(void *ptr);
void WeatherTask(void *ptr);
void KeyTask(void *ptr);
void DisplayTask(void *ptr);
void Timer1Callback(TimerHandle_t xTimer);

#endif
