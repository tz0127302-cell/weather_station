/**
 * @file weather_voice.c
 * @brief 天气预报语音播报模块
 * @details 通过MY1680语音模块播报当前天气、温度、湿度
 *         MY1680支持多级目录文件选择(通过文件夹编号和文件名寻址)
 *         音频文件预存在TF卡中，通过UART通信控制播放
 * @author He
 * @date 2026-05-06
 */

#include "weather_voice.h"

/**
 * @brief 播报数值，支持浮点数(自动分离整数和小数部分)
 * @param num 要播报的浮点数值，例如温度26.5、湿度80.0等
 * @note   处理逻辑：
 *         1. 分离整数部分和小数部分(保留一位小数，四舍五入)
 *         2. 小数部分先进位处理：9.99 -> 10.0，即小数为10则整数进1
 *         3. 整数值上限为100
 *         4. 播报整数部分：10~99按十位+十+个位方式播报
 *                         0~9或100直接播报
 *         5. 播报小数部分：先播"点"再播小数位数字
 */
static void MY1680_PlayNum(float num)
{
    int int_part = (int)num;
    int frac_part = (int)((num - int_part) * 10 + 0.5f);

    if(frac_part >= 10)
    {
        frac_part = 0;
        int_part++;
    }
    if(int_part > 100) int_part = 100;

    if(int_part >= 10 && int_part <= 99)
    {
        MY1680_Play(0x00, int_part / 10);
        MY1680_Play(0x00, 10);
        if(int_part % 10 != 0)
            MY1680_Play(0x00, int_part % 10);
    }
    else
    {
        MY1680_Play(0x00, int_part);
    }

    if(frac_part > 0)
    {
        MY1680_Play(0x00, 15);
        MY1680_Play(0x00, frac_part);
    }
}

/**
 * @brief 等待MY1680播放完成
 * @param timeout_ms 超时等待时间（毫秒）
 * @note   BUSY引脚为高电平=空闲，低电平=忙(正在播放)
 *         同时检测voice_abort标志，收到中止指令立即返回
 */
static void Wait_Play_Done(u32 timeout_ms)
{
    u32 cnt = 0;
    while (cnt < timeout_ms)
    {
        if (voice_abort) return;

        if (MY1680_BUSY != Bit_RESET)    /* BUSY高电平=空闲 */
        {
            return;
        }
        delay_ms(1);
        cnt++;
    }
}

/**
 * @brief 播报中止处理: 恢复UART4中断并清除忙标志
 * @note  语音播报期间UART4中断被禁用，任何提前退出都必须调用此函数
 */
static void VoicePlay_Exit(void)
{
    printf("[VOICE] playback end, re-enabling IRQ\r\n");
    delay_ms(300);             /* 等待扬声器尾音消散 */
    SU03T_EnableIRQ();         /* 恢复中断并启动2秒冷却期，冷却结束后自动清除voice_busy */
}

/* ================================================================
 * 语音播报保护策略：
 * - 天气/时间/温湿度播报期间：禁用UART4中断（硬件级屏蔽）
 *   SU-03T的UART数据无法触发ISR，从源头杜绝误识别。
 * - 音乐播放期间：使用voice_busy软件标志（允许用户切歌/停止）
 *   音乐播放时间长，必须保留语音控制能力。
 * ================================================================ */

/**
 * @brief 天气预报语音播报主函数
 * @param weather_file 天气状况对应的音频文件号(在03号文件夹中)
 * @param temp 当前温度值(摄氏度)
 * @param hum 当前湿度值(百分比)
 * @param city_file 城市名称对应的音频文件号(在02号文件夹中)
 * @note   播报期间禁用UART4中断，从硬件层面防止播报音频被误识别。
 */
void Weather_Voice_Play(u8 weather_file, int temp, u16 hum, u8 city_file)
{
    printf("[VOICE] Weather_Voice_Play start\r\n");
    SU03T_DisableIRQ();
    voice_busy = 1;
    voice_abort = 0;
    su03t.flag = 0;
    su03t.len = 0;

    /* 先播报一次城市名 */
    MY1680_Play(0x02, city_file);
    Wait_Play_Done(3000);
    if (voice_abort) { VoicePlay_Exit(); return; }

    /* 1. 播报 "目前" */
    MY1680_Play(0x02, city_file);
    Wait_Play_Done(3000);
    if (voice_abort) { VoicePlay_Exit(); return; }

    /* 2. 播报天气状况 */
    if (weather_file < 255)
    {
        MY1680_Play(0x03, weather_file);
        Wait_Play_Done(3000);
        if (voice_abort) { VoicePlay_Exit(); return; }
    }

    /* 3. 播报"温度"前缀 */
    MY1680_Play(0x00, 14);
    Wait_Play_Done(3000);
    if (voice_abort) { VoicePlay_Exit(); return; }

    /* 4. 播报温度数值 */
    MY1680_PlayNum((float)temp);
    Wait_Play_Done(3000);
    if (voice_abort) { VoicePlay_Exit(); return; }

    /* 5. 播报 "度" */
    MY1680_Play(0x00, 11);
    Wait_Play_Done(3000);
    if (voice_abort) { VoicePlay_Exit(); return; }

    /* 6. 播报湿度前缀 */
    MY1680_Play(0x00, 24);
    Wait_Play_Done(3000);
    if (voice_abort) { VoicePlay_Exit(); return; }

    /* 7. 播报湿度数值 */
    MY1680_PlayNum((float)hum);
    Wait_Play_Done(3000);

    VoicePlay_Exit();        /* 恢复UART4中断 + 清除忙标志 */
}

/**
 * @brief 播报当前RTC时间：XXXX年X月X日X点X分
 * @note  播报期间禁用UART4中断。
 */
void Voice_ReportTime(void)
{
    SU03T_DisableIRQ();
    voice_busy = 1;
    voice_abort = 0;
    su03t.flag = 0;
    su03t.len = 0;

    RTC_Analysis();

    /* === 播报年份 === */
    u16 y = rtc_time.year;
    MY1680_Play(0x00, y / 1000);
    Wait_Play_Done(3000);
    if (voice_abort) { VoicePlay_Exit(); return; }
    MY1680_Play(0x00, (y / 100) % 10);
    Wait_Play_Done(3000);
    if (voice_abort) { VoicePlay_Exit(); return; }
    MY1680_Play(0x00, (y / 10) % 10);
    Wait_Play_Done(3000);
    if (voice_abort) { VoicePlay_Exit(); return; }
    MY1680_Play(0x00, y % 10);
    Wait_Play_Done(3000);
    if (voice_abort) { VoicePlay_Exit(); return; }
    MY1680_Play(0x00, 15);                    /* "年" */
    Wait_Play_Done(3000);
    if (voice_abort) { VoicePlay_Exit(); return; }

    /* === 播报月份 === */
    MY1680_PlayNum((float)rtc_time.month);
    Wait_Play_Done(3000);
    if (voice_abort) { VoicePlay_Exit(); return; }
    MY1680_Play(0x00, 16);                    /* "月" */
    Wait_Play_Done(3000);
    if (voice_abort) { VoicePlay_Exit(); return; }

    /* === 播报日期 === */
    MY1680_PlayNum((float)rtc_time.day);
    Wait_Play_Done(3000);
    if (voice_abort) { VoicePlay_Exit(); return; }
    MY1680_Play(0x00, 17);                    /* "日" */
    Wait_Play_Done(3000);
    if (voice_abort) { VoicePlay_Exit(); return; }

    /* === 播报小时 === */
    MY1680_PlayNum((float)rtc_time.hour);
    Wait_Play_Done(3000);
    if (voice_abort) { VoicePlay_Exit(); return; }
    MY1680_Play(0x00, 20);                    /* "点" */
    Wait_Play_Done(3000);
    if (voice_abort) { VoicePlay_Exit(); return; }

    /* === 播报分钟 === */
    MY1680_PlayNum((float)rtc_time.minute);
    Wait_Play_Done(3000);
    if (voice_abort) { VoicePlay_Exit(); return; }
    MY1680_Play(0x00, 21);                    /* "分" */
    Wait_Play_Done(3000);

    VoicePlay_Exit();
}

/**
 * @brief 播报室内温度
 * @note  播报期间禁用UART4中断。
 */
void Voice_ReportIndoorTemp(void)
{
    SU03T_DisableIRQ();
    voice_busy = 1;
    voice_abort = 0;
    su03t.flag = 0;
    su03t.len = 0;

    MY1680_Play(0x00, 12);                    /* "室内温度" */
    Wait_Play_Done(3000);
    if (voice_abort) { VoicePlay_Exit(); return; }
    MY1680_PlayNum((float)indoor_data.temp);
    Wait_Play_Done(3000);
    if (voice_abort) { VoicePlay_Exit(); return; }
    MY1680_Play(0x00, 11);                    /* "度" */
    Wait_Play_Done(3000);

    VoicePlay_Exit();
}

/**
 * @brief 播报室内湿度
 * @note  播报期间禁用UART4中断。
 */
void Voice_ReportIndoorHum(void)
{
    SU03T_DisableIRQ();
    voice_busy = 1;
    voice_abort = 0;
    su03t.flag = 0;
    su03t.len = 0;

    MY1680_Play(0x00, 13);                    /* "室内湿度" */
    Wait_Play_Done(3000);
    if (voice_abort) { VoicePlay_Exit(); return; }
    MY1680_PlayNum((float)indoor_data.humi);
    Wait_Play_Done(3000);
    if (voice_abort) { VoicePlay_Exit(); return; }
    MY1680_Play(0x00, 23);                    /* "百分之" */
    Wait_Play_Done(3000);

    VoicePlay_Exit();
}
