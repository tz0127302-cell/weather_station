#include "app.h"


TaskHandle_t Task1Handle;
TaskHandle_t Task2Handle;
TaskHandle_t Task3Handle;
extern TaskHandle_t StartTaskHandle;
QueueHandle_t queue1;
TimerHandle_t Timer1Handle;

void StartTask(void *ptr)
{
    printf("StartTask is running\r\n");
   xTaskCreate(Task1,    // 任务函数指针
                "Task1",    // 任务名称
                START_TASK_STACK_SIZE,    // 任务栈大小
                "hello task1",  // 任务参数
                3,  // 任务优先级
                &Task1Handle );  // 任务句柄  
    xTaskCreate(Task2,    // 任务函数指针
                "Task2",    // 任务名称 
                START_TASK_STACK_SIZE,    // 任务栈大小
                "hello task2",  // 任务参数
                2,  // 任务优先级
                &Task2Handle );  // 任务句柄  
    xTaskCreate(Task3,    // 任务函数指针
                "Task3",    // 任务名称
                START_TASK_STACK_SIZE,    // 任务栈大小
                "hello task3",  // 任务参数
                1,  // 任务优先级
                &Task3Handle );  // 任务句柄  
    

    //创建队列
    queue1 = xQueueCreate(10,           // 队列长度
                         sizeof(u8));  // 队列中每个元素的大小
    //创建定时器1
    Timer1Handle = xTimerCreate("Timer1", 1000 / portTICK_PERIOD_MS, pdTRUE, NULL, Timer1Callback);
    if (Timer1Handle != NULL)//启动定时器1
    {
        xTimerStart(Timer1Handle, 0);//
    }

    //删除起始任务
    vTaskDelete(StartTaskHandle);
}

void Task1(void *ptr)
{
    printf("Task1 is running\r\n");
    while(1)
    {     
         //等待队列中的数据，参数：队列句柄、接收数据的缓冲区、等待时间
         u8 key;
         if(xQueueReceive(queue1, &key, portMAX_DELAY) == pdPASS)  //从队列接收数据，参数：队列句柄、接收数据的缓冲区、等待时间
         {
             printf("Task1 received key from queue1: %d\r\n", key);
             LED1_TOGGLE;
             LED2_TOGGLE;
         }
         else
         {
             printf("Task1 receive key from queue1 failed\r\n");
         }
        vTaskDelay(20);  // 延时20个系统节拍
        // //挂起任务1
        // vTaskSuspend(Task1Handle);
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
            if(xQueueSend( queue1,&key,0) == pdPASS)  //发送数据到队列，参数：队列句柄、数据指针、等待时间
            {
                printf("Task1 send key to queue1\r\n");
            }
            else
            {
                printf("Task1 send key to queue1 failed\r\n");
            }
            
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




