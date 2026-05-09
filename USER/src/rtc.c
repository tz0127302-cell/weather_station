/**
 * @file rtc.c
 * @brief RTC实时时钟函数源文件
 * @details 包含RTC初始化、闰年判断、日期时间设置、秒计数器解析等功能
 *          使用HSE/128作为RTC时钟源，通过分频得到1Hz的RTC时钟
 *          支持从1970年1月1日开始的秒计数与日期时间的相互转换
 * @author He
 * @date 2026-04-25
 */

#include "rtc.h"

/* 全局RTC时间结构体变量，用于存储解析后的日期和时间信息 */
RTC_TimeTypeDef rtc_time;



/**
  * @brief RTC初始化函数
  * @param  void
  * @retval void
  * @details 配置RTC时钟源为HSE/128（8MHz/128=62.5KHz），
  *          设置预分频器为62500，使最终RTC计数频率为1Hz（62.5KHz/62500=1Hz）。
  *          根据ntp_flag标志决定使用手动设定时间还是NTP网络时间：
  *          - ntp_flag=0：手动设置初始时间（2026年4月30日 09:52:10）
  *          - ntp_flag=1：使用NTP获取的时间戳（需加8小时北京时区偏移）
  * @author He
  */
void Rtc_Init(void)
{
    /* 使能PWR（电源管理）时钟，访问备份寄存器需要先使能PWR */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    /* 使能备份域访问权限，允许对RTC和备份寄存器进行读写操作 */
    PWR_BackupAccessCmd(ENABLE);

    /* === RTC时钟源配置 === */
    /* 选择HSE时钟经128分频后作为RTC时钟源，HSE=8MHz，分频后=62.5KHz */
    RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);
    /* 使能RTC时钟 */
    RCC_RTCCLKCmd(ENABLE);
    /* 等待RTC寄存器上次操作完成 */
    RTC_WaitForLastTask();
    /* 等待RTC时钟同步完成，确保读写时序正确 */
    RTC_WaitForSynchro();
    /* 再次等待RTC寄存器操作完成 */
    RTC_WaitForLastTask();

    /* === RTC配置模式 === */
    /* 进入RTC配置模式，允许修改预分频器和计数器等寄存器 */
    RTC_EnterConfigMode();
    /* 设置预分频器值为62500：62.5KHz / 62500 = 1Hz，即RTC计数器每秒加1 */
    RTC_SetPrescaler(62500);
    /* 等待配置完成 */
    RTC_WaitForLastTask();

    /* === 时间初始化 === */
    /* 根据ntp_flag标志位选择时间设置方式 */
    if(ntp_flag == 0)
        /* ntp_flag=0：手动设置初始时间（2026年4月30日 09:52:10） */
        RTC_SetTimeandDate(9,52,10,30,4,2026);
    else if(ntp_flag == 1)
        /* ntp_flag=1：使用NTP服务器获取的时间戳，加8小时（北京时间=UTC+8） */
        RTC_SetCounter(timestamp + (8 * 3600));

    /* 等待RTC操作完成 */
    RTC_WaitForLastTask();
    /* 退出RTC配置模式，RTC开始正常运行 */
    RTC_ExitConfigMode();
}


/**
  * @brief 闰年判断函数
  * @param  u16 year - 要判断的年份（如2026）
  * @retval u8 返回1表示闰年，返回0表示平年
  * @details 闰年判断规则：能被4整除但不能被100整除，或者能被400整除
  *          闰年2月有29天，平年2月有28天
  * @author He
  */
u8 Leap_Year(u16 year)
{
    /* 闰年条件：能被4整除且不能被100整除，或者能被400整除 */
    if(year%4==0&&year%100!=0||year%400==0)
    {
        return 1;  /* 是闰年 */
    }
    return 0;      /* 是平年 */
}

/**
  * @brief 设置RTC日期时间（通过秒计数器方式）
  * @param  u8 hour   - 小时 (0-23)
  * @param  u8 minute - 分钟 (0-59)
  * @param  u8 second - 秒钟 (0-59)
  * @param  u8 day    - 日 (1-31)
  * @param  u8 month  - 月 (1-12)
  * @param  u16 year  - 年 (1970+)
  * @retval void
  * @details 将输入的日期时间转换为从1970年1月1日00:00:00开始的秒计数，
  *          然后调用RTC_SetCounter()写入RTC计数器。
  *          转换过程依次处理：年累计秒数、月累计秒数、日累计秒数、时分秒累计秒数
  * @author He
  */
void RTC_SetTimeandDate(u8 hour,u8 minute,u8 second,u8 day,u8 month,u16 year)
{
    u16 i;
    u32 second_cnt = 0;
    /* 打印当前要设置的日期时间，用于调试确认 */
    printf("year: %d  month: %d  day: %d  hour: %d  minute: %d  second: %d\r\n",year,month,day,hour,minute,second);

    /* === 计算从1970年到目标年份的总秒数 === */
    for(i=1970;i<year;i++)  /* 遍历1970年到year-1年 */
    {
        if(Leap_Year(i))
            second_cnt += 31622400;  /* 闰年：366天 * 86400秒/天 */
        else
            second_cnt += 31536000;  /* 平年：365天 * 86400秒/天 */
    }

    /* 使用局部临时数组处理月份天数，避免影响全局数据 */
    /* 定义各月份的天数（平年），2月默认为28天 */
    u8 temp_months_day[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    /* 如果目标年份是闰年，将2月天数改为29天 */
    if(Leap_Year(year))
    {
        temp_months_day[1] = 29;  /* 闰年2月为29天 */
    }

    /* === 计算从1月到目标月份的前一个月的总秒数 === */
    for(i=0;i<month-1;i++)
    {
        second_cnt += temp_months_day[i]*86400;  /* 每月天数 * 86400秒/天 */
    }

    /* === 计算日、时、分、秒的累计秒数 === */
    second_cnt += (day-1)*86400;  /* 日偏移（从当月1日开始计数，所以减1） */
    second_cnt += hour*3600;       /* 小时转秒 */
    second_cnt += minute*60;       /* 分钟转秒 */
    second_cnt += second;          /* 秒 */

    /* 打印计算出的总秒数，用于调试验证 */
    printf("second_cnt: %d\r\n",second_cnt);
    /* 将总秒数写入RTC计数器，RTC硬件将以此值开始计数 */
    RTC_SetCounter(second_cnt);
}



/**
  * @brief RTC时间日期解析函数
  * @param  void
  * @retval void
  * @details 从RTC计数器读取当前秒计数，将其转换为可读的日期和时间信息。
  *          转换算法：先通过天数计算出年份，再根据剩余天数计算月份和日，
  *          最后从总秒数中提取时、分、秒。
  *          结果存储在全局变量rtc_time结构体中，并通过串口打印输出
  * @author He
  */
void RTC_Analysis(void)
{
    /* 从RTC外设寄存器中读取当前的秒计数 */
    u32 temp_second = RTC_GetCounter();
    /* 计算自1970年1月1日以来经过的总天数 */
    u32 day_count = temp_second/86400; /* 86400 = 24小时 * 3600秒/小时 */

    /* === 通过逐次减法计算当前年份 === */
    u32 year = 1970;
    while(1)
    {
        /* 获取当前年份的天数（闰年366天，平年365天） */
        u32 days_in_year = Leap_Year(year) ? 366 : 365;
        /* 如果剩余天数大于等于当前年份的天数，则减去该年天数，年份+1 */
        if(day_count >= days_in_year)
        {
            day_count -= days_in_year;
            year++;
        } else
        {
            break;  /* 剩余天数不足一年，说明当前就在此年份 */
        }
    }

    /* === 根据闰年判断结果确定每个月的天数 === */
    u8 temp_months_day[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    /* 如果当前年份是闰年，将2月天数改为29天 */
    if(Leap_Year(year))
        temp_months_day[1] = 29;

    /* === 通过逐次减法计算当前月份和日期 === */
    u32 month = 0;
    /* 从1月开始，如果剩余天数大于等于当月天数，则减去当月天数，月份+1 */
    while(month < 11 && day_count >= temp_months_day[month])
    {  /* 月份边界保护：最多循环到第11个月（12月） */
        day_count -= temp_months_day[month];
        month++;
    }
    month += 1;  /* month从0开始计数，转为实际月份（1-12） */

    /* 日从1开始计数，day_count经过逐月减法后剩余的就是当月第几天减1 */
    u32 day = day_count + 1;
    /* 从总秒数中提取小时（总秒数/3600再对24取余） */
    u32 hour = temp_second/3600%24;
    /* 从总秒数中提取分钟（总秒数/60再对60取余） */
    u32 minute = temp_second/60%60;
    /* 从总秒数中提取秒钟（总秒数对60取余） */
    u32 second = temp_second%60;

    /* 通过串口打印解析出的时间和日期信息 */
    printf("Time: %02d:%02d:%02d\r\n",hour,minute,second);
    printf("Date: %02d-%02d-%04d\r\n",day,month,year);

    /* 将解析结果存储到全局RTC时间结构体中，供其他模块使用 */
    rtc_time.year   = year;
    rtc_time.month  = month;
    rtc_time.day    = day;
    rtc_time.hour   = hour;
    rtc_time.minute = minute;
    rtc_time.second = second;
}
