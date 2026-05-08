/**
 * @file delay.c
 * @brief 软件延时函数源文件
 * @details 实需责微秒级和毫秒级的延时功能
 * @author He
 * @date 2026-04-25
 */

#include "delay.h"

/*
函数功能：微秒延时
形参：u32 nus    需要延时多少微秒
返回值：void
函数说明：

unsigned int     --    u32
unsigned short   --    u16
unsigned char    --    u8      

主频：72Mhz
500ns = 0.5us     800ns  = 0.8us
*/

void delay_us(u32 nus)
{
	
	while(nus--)
	{
		//nus每自减一次需要卡主CPU一微秒   72个__NOP(); 
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
        __NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
        __NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
        __NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
        __NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
        __NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
        __NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
        __NOP();__NOP();
	}
	
}


/*
函数功能：毫秒延时
形参：u32 nms    需要延时多少毫秒
返回值：void
函数说明：

*/

/**
 * @brief 毫秒级软件延时函数
 * @param u32 nms - 需要稍延执行的时间（单位：毫秒）
 * @retval void
 * @details 五调用delay_us()函数实现1毫秒的延时效果
 */
void delay_ms(u32 nms)
{
    /* 每次次循环每次稍延1毫秒(1000微秒) */
    while(nms--)
    {
        delay_us(1000);    /* 每个毫秒需要1000个微秒 */
    }
}




