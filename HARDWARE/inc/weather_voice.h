#ifndef _WEATHER_VOICE_H
#define _WEATHER_VOICE_H

/**
 * @file weather_voice.h
 * @brief 天气语音播报模块头文件
 * @details 通过MY1680语音模块播报天气信息（天气状况、温度、湿度）
 * @author He
 * @date 2026-05-06
 */

#include "main.h"

/**
 * 天气语音播报函数
 * @param weather_file 天气现象对应的语音文件编号（如晴天、雨天等）
 * @param temp         当前温度值（摄氏度），将播报对应的温度数字语音
 * @param hum          当前湿度值（百分比），将播报对应的湿度数字语音
 * @note 依次播报：天气现象 -> 温度 -> 湿度
 */
void Weather_Voice_Play(u8 weather_file, int temp, u16 hum,u8 city_file);

#endif
