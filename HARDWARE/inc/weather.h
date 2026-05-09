#ifndef _WEATHER_H
#define _WEATHER_H

/**
 * @file weather.h
 * @brief 天气数据处理模块头文件
 * @details 定义天气数据结构体和JSON解析、LCD显示函数声明
 * @author He
 * @date 2026-05-06
 */

#include "main.h"

/* 天气数据结构体 */
typedef struct
{
    char cityName[16];    /* 城市名称：当前查询的城市名字符串 */
    char tq[16];          /* 天气现象描述：如"晴"/"多云"/"雨"等天气状况字符串 */
    int  qw;              /* 当前温度：单位为摄氏度(℃) */
    int  sd;              /* 相对湿度：单位为百分比(%) */
    char fl[16];          /* 风力等级：如"1级"/"2级"等风力描述字符串 */
    char fx[16];          /* 风向描述：如"东风"/"南风"等风向字符串 */
    u8   voice_file;      /* 语音播报对应文件编号：根据天气现象匹配的语音文件序号(numtq) */
}WeatherData;

/* 函数声明 */
void Weather_Parse(char *json, WeatherData *w);           /* JSON解析函数：从天气API返回的JSON字符串中提取数据，填充到WeatherData结构体中 */
void Weather_Display(const WeatherData *w);               /* LCD显示函数：将WeatherData中的天气信息显示到LCD屏幕上 */

#endif
