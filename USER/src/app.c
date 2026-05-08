#include "app.h"

/*任务句柄*/
TaskHandle_t Task1Handle;
TaskHandle_t Task2Handle;
TaskHandle_t Task3Handle;
/*起始任务句柄*/
extern TaskHandle_t StartTaskHandle;
/*队列句柄*/
QueueHandle_t queue1;
/*定时器句柄*/
TimerHandle_t Timer1Handle;
/*信号量句柄*/
SemaphoreHandle_t sem1;//二值信号量

void StartTask(void *ptr)
{
    printf("StartTask is running\r\n");
   xTaskCreate(Task1,                        // 任务函数指针
                "Task1",                     // 任务名称
                START_TASK_STACK_SIZE,       // 任务栈大小
                "hello task1",               // 任务参数
                3,                           // 任务优先级
                &Task1Handle );              // 任务句柄  
    xTaskCreate(Task2,                       // 任务函数指针
                "Task2",                     // 任务名称 
                START_TASK_STACK_SIZE,       // 任务栈大小
                "hello task2",               // 任务参数
                2,                           // 任务优先级
                &Task2Handle );              // 任务句柄  
    xTaskCreate(Task3,                       // 任务函数指针
                "Task3",                     // 任务名称
                START_TASK_STACK_SIZE,       // 任务栈大小
                "hello task3",               // 任务参数
                1,                           // 任务优先级
                &Task3Handle );              // 任务句柄  
    

    //创建队列
    queue1 = xQueueCreate(10,           // 队列长度
                         sizeof(u8));  // 队列中每个元素的大小
    //创建定时器1
    Timer1Handle = xTimerCreate("Timer1", 1000 / portTICK_PERIOD_MS, pdTRUE, NULL, Timer1Callback);
    if (Timer1Handle != NULL)
    {
        xTimerStart(Timer1Handle, 0);//启动定时器1
    }
    //创建二值信号量
    sem1 = xSemaphoreCreateBinary();
    if (sem1 != NULL)
    {
        printf("xSemaphoreCreateBinary success\r\n");
    }
    //删除起始任务
    vTaskDelete(StartTaskHandle);
}

void Task1(void *ptr)
{
    printf("Task1 is running\r\n");
    while(1)
    {     //等待信号量
         if(xSemaphoreTake(sem1,portMAX_DELAY) == pdTRUE)//1->0成功
        {
            printf("run Task1\r\n");
            LED1_TOGGLE;
            LED2_TOGGLE;
        }
    }
}

void Task2(void *ptr)
{
    u8 key;
    while(1)
    {
        key = Key_Scan();
        if(key == 1)
        {
           //释放信号量
            xSemaphoreGive(sem1);
            printf("Task2 release sem to Task1\r\n");
            
        }

    }
}

void Task3(void *ptr)
{
    while(1)
    {
        printf("Task3 is running\r\n");
        // vTaskDelay(1000 / portTICK_PERIOD_MS);
        delay_ms(1000);
        //恢复任务1
       
    }
}

//定时器1回调函数
void Timer1Callback(TimerHandle_t xTimer)
{
    uint8_t temp, humi;
    if (DHT11_Read_Data(&temp, &humi) == 0)
    {
        printf("DHT11: Temp=%dC, Humi=%d%%\r\n", temp, humi);
    }
    else
    {
        printf("DHT11 read failed\r\n");
    }
}




