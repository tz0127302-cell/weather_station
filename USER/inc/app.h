#ifndef _APP_H
#define _APP_H

#include "main.h"

/*
 * FreeRTOS任务配置宏定义
 * START_TASK_STACK_SIZE: 启动任务的栈空间大小（单位：字）
 *                        该任务负责创建其他所有任务，栈大小需足够
 * START_TASK_PRIORITY:  启动任务的优先级（数值越大优先级越高）
 *                        此处设为4，为中等优先级
 */
#define START_TASK_STACK_SIZE 256
#define START_TASK_PRIORITY  4

/*
 * 函数声明：FreeRTOS任务及回调函数
 *
 * StartTask      - 系统启动任务，负责创建WeatherTask、KeyTask、DisplayTask等其他任务
 *                  参数 ptr: 传递给任务的用户参数（通常为NULL）
 *                  返回值: 无（任务函数不返回）
 *
 * WeatherTask    - 气象数据采集任务，负责读取传感器数据并更新气象信息
 *                  参数 ptr: 传递给任务的用户参数（通常为NULL）
 *                  返回值: 无（任务函数不返回）
 *
 * KeyTask        - 按键扫描任务，负责检测用户按键输入并执行相应操作
 *                  参数 ptr: 传递给任务的用户参数（通常为NULL）
 *                  返回值: 无（任务函数不返回）
 *
 * DisplayTask    - 显示刷新任务，负责将当前数据更新到LCD屏幕
 *                  参数 ptr: 传递给任务的用户参数（通常为NULL）
 *                  返回值: 无（任务函数不返回）
 *
 * Timer1Callback - 定时器1回调函数，由FreeRTOS软件定时器触发
 *                  参数 xTimer: 触发回调的定时器句柄
 *                  返回值: 无
 */
void StartTask(void *ptr);
void WeatherTask(void *ptr);
void KeyTask(void *ptr);
void DisplayTask(void *ptr);
void Timer1Callback(TimerHandle_t xTimer);

#endif
