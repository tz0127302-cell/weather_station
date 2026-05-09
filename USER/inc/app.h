#ifndef _APP_H
#define _APP_H

#include "main.h"



#define START_TASK_STACK_SIZE 256
#define START_TASK_PRIORITY  4/*  */


void StartTask(void *ptr);
void WeatherTask(void *ptr);
void KeyTask(void *ptr);
void DisplayTask(void *ptr);
void Timer1Callback(TimerHandle_t xTimer);



#endif