#ifndef __DHT11_H
#define __DHT11_H

#include "main.h"

#define DHT11_DQ_OUT_H (GPIO_SetBits(GPIOC, GPIO_Pin_1))
#define DHT11_DQ_OUT_L (GPIO_ResetBits(GPIOC, GPIO_Pin_1))
#define DHT11_DQ_IN    (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1))

// 函数声明
uint8_t DHT11_Init(void);
uint8_t DHT11_Read_Data(uint8_t *temp, uint8_t *humi);

#endif
