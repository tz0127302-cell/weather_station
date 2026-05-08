#ifndef _WEATHER_H
#define _WEATHER_H

/**
 * @file weather.h
 * @brief 天气数据处理模块头文件
 * @details 包含天气数据结构体和解析、显示函数声明
 * @author He
 * @date 2026-05-06
 */

#include "main.h"

/* 天气数据结构体 */
typedef struct
{
    char cityName[16];    /* 城市名称 */
    char tq[16];          /* 天气描述 (晴/多云/雨) */
    int  qw;              /* 当前温度 (C) */
    int  sd;              /* 相对湿度 (%) */
    char fl[16];          /* 风力等级 */
    char fx[16];          /* 风向描述 */
    u8   voice_file;      /* 语音对应文件编号(numtq) */
}WeatherData;

/* 函数声明 */
void Weather_Parse(char *json, WeatherData *w);           /* 解析JSON天气数据 */
void Weather_Display(const WeatherData *w);               /* LCD显示天气信息 */

#endif
