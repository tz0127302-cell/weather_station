/**
 * @file weather_voice.c
 * @brief 天气语音播报模块
 * @details 通过MY1680语音模块播报当前天气、温度、湿度
 *         MY1680支持多路语音文件选择(通过文件夹编号和文件编号寻址)
 *         语音文件预存在TF卡中，通过UART通信控制播放
 * @author He
 * @date 2026-05-06
 */

#include "weather_voice.h"

/**
 * @brief 播报数值（支持浮点数，自动处理整数部分和小数部分）
 * @param num 要播报的浮点数值（如温度26.5、湿度80.0等）
 * @note   处理逻辑：
 *         1. 分离整数部分和小数部分(保留一位小数，四舍五入)
 *         2. 处理精度进位：如9.99 -> 10.0，修正为整数部分10，小数部分0
 *         3. 限制最大值为100
 *         4. 播报整数部分：10~99按十位+十+个位方式播报
 *                         0~9或100直接播报
 *         5. 如有小数部分，播报"点"后跟小数位数字
 *
 *         MY1680的0号文件夹(0x00)存放数字语音文件：
 *         文件编号0-9对应数字0-9
 *         文件编号10对应"十"
 *         文件编号11对应"度"(温度单位)
 *         文件编号14对应"温度"前缀
 *         文件编号15对应"点"(小数点)
 *         文件编号23对应"百分比"(湿度单位)
 */
static void MY1680_PlayNum(float num)
{
    // 分离整数部分和小数部分(保留一位小数，加0.5实现四舍五入)
    int int_part = (int)num;
    int frac_part = (int)((num - int_part) * 10 + 0.5f);

    // 处理小数进位情况：如果小数部分>=10，说明发生了进位(如9.99 -> 10.0)
    if(frac_part >= 10)
    {
        frac_part = 0;       // 小数部分归零
        int_part++;           // 整数部分加1
    }
    if(int_part > 100) int_part = 100;  // 限制最大值不超过100

    // 播报整数部分
    if(int_part >= 10 && int_part <= 99)
    {
        // 播报十位数字："几十"
        MY1680_Play(0x00, int_part / 10);
        // 播报"十"
        MY1680_Play(0x00, 10);
        // 如果个位不为0，播报个位数字(如"三十五"的"五")
        if(int_part % 10 != 0)
            MY1680_Play(0x00, int_part % 10);
    }
    else
    {
        // 0~9 或 100 直接播报对应编号的文件
        MY1680_Play(0x00, int_part);
    }

    // 播报小数部分(如果有)
    if(frac_part > 0)
    {
        MY1680_Play(0x00, 15);          // 播报"点"
        MY1680_Play(0x00, frac_part);   // 播报小数位数字
    }
}

/**
 * @brief 等待MY1680播报完成
 * @param timeout_ms 最大等待超时时间（毫秒）
 * @note   通过检测MY1680的BUSY引脚状态来判断是否播放完成
 *         当BUSY引脚为高电平表示繁忙(正在播放)，低电平表示空闲
 *         如果超过超时时间仍未完成，强制返回避免死等
 */
static void Wait_Play_Done(u32 timeout_ms)
{
    u32 cnt = 0;
    while (cnt < timeout_ms)
    {
        if (MY1680_BUSY != Bit_RESET)   /* BUSY引脚为高电平(RESET=0)，表示还在播放中 */
        {
            return;          /* 播放完成(或超时)，返回 */
        }
        delay_ms(1);
        cnt++;
    }
}

/**
 * @brief 天气语音播报总函数
 * @param weather_file 天气状况对应的语音文件编号(在03号文件夹中)
 * @param temp 当前温度值(摄氏度)
 * @param hum 当前湿度值(百分比)
 * @note   播报顺序：
 *         [可选] 播报唤醒音(第1步，当前被注释掉未启用)
 *         1. 播报"目前"前缀（02号文件夹，文件10）
 *         2. 播报天气状况（03号文件夹，weather_file编号）
 *           如果weather_file==255则不播报(无效编号)
 *         3. 播报"温度"（00号文件夹，文件14）
 *         4. 播报温度数值（调用MY1680_PlayNum）
 *         5. 播报"度"单位（00号文件夹，文件11）
 *         6. 播报"百分比"前缀（00号文件夹，文件23）
 *         7. 播报湿度数值（调用MY1680_PlayNum）
 *
 *         MY1680文件夹说明：
 *         00号文件夹：数字、单位等通用语音文件
 *         02号文件夹：短语片段(如"目前")
 *         03号文件夹：天气状况语音(如"晴"、"多云"、"阵雨"等)
 */
void Weather_Voice_Play(u8 weather_file, int temp, u16 hum)
{/* 先播放一个唤醒音(第一分区内的一段提示音，当前被注释未启用) */
    MY1680_Play(0x02, 10);
    Wait_Play_Done(3000);
    /* 1. 播放 "目前" */
    MY1680_Play(0x02, 10);
    Wait_Play_Done(3000);

    /* 2. 播放天气状况(如晴、多云、雨等) */
    if (weather_file < 255)              /* 255表示无效编号，跳过 */
    {
        MY1680_Play(0x03, weather_file);
        Wait_Play_Done(3000);
    }
    /* 3. 播放温度提示 */
    MY1680_Play(0x00, 14);               /* "温度" */
    Wait_Play_Done(3000);

    /* 4. 播放温度数值 */
    MY1680_PlayNum((float)temp);         /* 播报具体温度数字 */
    Wait_Play_Done(3000);

    /* 5. 播放 "度" */
    MY1680_Play(0x00, 11);               /* 温度单位"度" */
    Wait_Play_Done(3000);

    /* 6. 播放 "%" 湿度提示前缀 */
    MY1680_Play(0x00, 23);               /* "百分比" */
    Wait_Play_Done(3000);

    /* 7. 播放湿度数值 */
    MY1680_PlayNum((float)hum);          /* 播报具体湿度数字 */
    Wait_Play_Done(3000);
}
