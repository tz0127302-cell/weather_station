/**
 * @file rtc.c
 * @brief RTC实时时钟驱动源文件
 * @details 处理RTC初始化、时间设置、时间数据转换
 * @author He
 * @date 2026-04-25
 */

#include "rtc.h"




/**
  * @brief RTC初始化
  * @param  void
  * @retval void
  * @author He
    
  */


void Rtc_Init(void)
{
    //使能PWR时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    PWR_BackupAccessCmd(ENABLE);//使能备份域访问

    // 初始化RTC
    RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);//选择HSE/128作为RTC时钟
    RCC_RTCCLKCmd(ENABLE);//使能RTC时钟
    RTC_WaitForLastTask();//等待RTC最后任务完成
    RTC_WaitForSynchro();//等待RTC同步完成
    RTC_WaitForLastTask();//等待RTC最后任务完成

    RTC_EnterConfigMode();//进入配置模式
    RTC_SetPrescaler(62500);//8000000hz的HSE/128/62500=1hz的RTC时钟
    RTC_WaitForLastTask();//等待RTC最后任务完成
    if(ntp_flag == 0)
     RTC_SetTimeandDate(9,52,10,30,4,2026);
    else if(ntp_flag == 1)
     RTC_SetCounter(timestamp + (8 * 3600));//东八区 + 8小时
    RTC_WaitForLastTask();//等待RTC最后任务完成
    RTC_ExitConfigMode();//退出配置模式


}


/**
  * @brief 闰年平年判断
  * @param  void
  * @retval void
  * @author He
    
  */
u8 Leap_Year(u16 year)
{
    
    if(year%4==0&&year%100!=0||year%400==0)
    {
        return 1;
    }
    return 0;
   
}

/**
  * @brief 根据设置的时间日期设置RTC计数值
  * @param  void
  * @retval void
  * @author He 
  */
void RTC_SetTimeandDate(u8 hour,u8 minute,u8 second,u8 day,u8 month,u16 year)
{
    u16 i;
    u32 second_cnt = 0;
    printf("year: %d  month: %d  day: %d  hour: %d  minute: %d  second: %d\r\n",year,month,day,hour,minute,second);
    //计算1970年1月1日到设置时间日期的秒数
    for(i=1970;i<year;i++)//年差秒
    {
        if(Leap_Year(i))
            second_cnt += 31622400;  // 366*86400（一天的秒数）
        else
            second_cnt += 31536000;  // 365*86400
    }
    
    // 使用临时数组处理闰年，不影响全局数组
    //月差秒
    u8 temp_months_day[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if(Leap_Year(year)) {
        temp_months_day[1] = 29;  // 闰年二月为29天
    }
    
    for(i=0;i<month-1;i++) {
        second_cnt += temp_months_day[i]*86400;
    }
        
    second_cnt += (day-1)*86400;//日差秒
    second_cnt += hour*3600;
    second_cnt += minute*60;
    second_cnt += second;
    printf("second_cnt: %d\r\n",second_cnt);//计数值（时间戳）
    RTC_SetCounter(second_cnt);


}



/**
  * @brief 时间日期分析
  * @param  void
  * @retval void
  * @author He
    
  */

void RTC_Analysis(void)
{
    
    u32 temp_second = RTC_GetCounter();
    u32 day_count = temp_second/86400; // 总天数
    
    // 计算年份和剩余天数
    u32 year = 1970;
    while(1) {
        u32 days_in_year = Leap_Year(year) ? 366 : 365;
        if(day_count >= days_in_year) {
            day_count -= days_in_year;
            year++;
        } else {
            break;
        }
    }
    
    // 根据是否闰年来确定每月天数
    u8 temp_months_day[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if(Leap_Year(year))
        temp_months_day[1] = 29;
    
    // 计算月份和日期
    u32 month = 0;
    while(month < 11 && day_count >= temp_months_day[month]) {  // 修正边界条件
        day_count -= temp_months_day[month];
        month++;
    }
    month += 1;

    u32 day = day_count + 1; // 天数从1开始计   
    u32 hour = temp_second/3600%24;
    u32 minute = temp_second/60%60;
    u32 second = temp_second%60;
    
    printf("Time: %02d:%02d:%02d\r\n",hour,minute,second);
    printf("Date: %02d-%02d-%04d\r\n",day,month,year);
}