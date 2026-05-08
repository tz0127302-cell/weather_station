#ifndef _APP_H
#define _APP_H

#include "main.h"



#define START_TASK_STACK_SIZE 256
#define START_TASK_PRIORITY  4/* 最高优先级 */


void StartTask(void *ptr);
void Task1(void *ptr);
void Task2(void *ptr);
void Task3(void *ptr);
void Timer1Callback(TimerHandle_t xTimer);



#endif