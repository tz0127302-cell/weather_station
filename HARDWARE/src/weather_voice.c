/**
 * @file weather_voice.c
 * @brief 天气语音播报模块
 * @details 通过MY1680播报当前天气、温度、湿度
 * @author He
 * @date 2026-05-06
 */

#include "weather_voice.h"

/**
 * @brief 播报数值 (可复用)
 * @param num 要播报的浮点数
 */
static void MY1680_PlayNum(float num)
{
    // 四舍五入取一位小数
    int int_part = (int)num;
    int frac_part = (int)((num - int_part) * 10 + 0.5f);

    // 处理小数点后进位（如9.99 → 10.0）
    if(frac_part >= 10)
    {
        frac_part = 0;
        int_part++;
    }
    if(int_part > 100) int_part = 100;

    // 整数部分播报
    if(int_part >= 10 && int_part <= 99)
    {
        // 十位（如45 → "四"）
        MY1680_Play(0x00, int_part / 10);
        // "十"
        MY1680_Play(0x00, 10);
        // 个位（整十数不播个位，如40 → "四十"）
        if(int_part % 10 != 0)
            MY1680_Play(0x00, int_part % 10);
    }
    else
    {
        // 0~9 和 100 直接播报
        MY1680_Play(0x00, int_part);
    }

    // 小数部分播报
    if(frac_part > 0)
    {
        MY1680_Play(0x00, 15);                 // "点"
        MY1680_Play(0x00, frac_part);          // 小数位
    }
}

/**
 * @brief 等待MY1680播报完成
 * @param timeout_ms 最大等待时间（毫秒）
 */
static void Wait_Play_Done(u32 timeout_ms)
{
    u32 cnt = 0;
    while (cnt < timeout_ms)
    {
        if (MY1680_BUSY != Bit_RESET)
        {
            return;
        }
        delay_ms(1);
        cnt++;
    }
}

/**
 * @brief 播报天气
 * @param weather_file 天气文件编号（目录03）
 * @param temp 温度
 * @param hum 湿度
 */
void Weather_Voice_Play(u8 weather_file, int temp, u16 hum)
{
    /* 1. 播放 "目前是" */
    MY1680_Play(0x00, 22);
    Wait_Play_Done(3000);

    /* 2. 播放天气现象 */
    if (weather_file < 255)
    {
        MY1680_Play(0x03, weather_file);
        Wait_Play_Done(3000);
    }

    /* 3. 室外温度提示 */
    MY1680_Play(0x00, 14);
    Wait_Play_Done(3000);

    /* 4. 播报温度数值 */
    MY1680_PlayNum((float)temp);
    Wait_Play_Done(3000);

    /* 5. 播报 "度" */
    MY1680_Play(0x00, 11);
    Wait_Play_Done(3000);

    /* 6. 播报 "%" */
    MY1680_Play(0x00, 23);
    Wait_Play_Done(3000);

    /* 7. 播报湿度数值 */
    MY1680_PlayNum((float)hum);
    Wait_Play_Done(3000);
}
