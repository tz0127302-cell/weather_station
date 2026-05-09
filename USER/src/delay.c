/**
 * @file delay.c
 * @brief 软件延时函数源文件
 * @details 实现了微秒级和毫秒级的软件延时功能，通过空指令(NOP)循环占用CPU时间实现精确延时
 * @author He
 * @date 2026-04-25
 */

#include "delay.h"

/*
 * 函数功能：微秒延时
 * 参数：u32 nus    需要延时的微秒数
 * 返回值：void
 * 类型说明：
 *   unsigned int     --    u32
 *   unsigned short   --    u16
 *   unsigned char    --    u8
 * 主频：72MHz
 * 时间计算：500ns = 0.5us，800ns = 0.8us
 * 实现原理：每循环一次执行一组__NOP()空指令，占用约1微秒的CPU时间
 *           72MHz主频下，每个__NOP()约为13.9ns，72个NOP约1us
 */

void delay_us(u32 nus)
{
    /* 循环nus次，每次消耗约1微秒 */
    while(nus--)
    {
        /* nus每自减一次，需要占用CPU约一微秒时间（72MHz下约72个NOP指令周期） */
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
 * 函数功能：毫秒延时
 * 参数：u32 nms    需要延时的毫秒数
 * 返回值：void
 * 实现原理：循环调用delay_us(1000)，每次延时1毫秒
 *           总延时时间 = nms * 1000微秒 = nms毫秒
 */

/**
 * @brief 毫秒级阻塞延时函数
 * @param u32 nms - 需要阻塞执行的延时时长（单位：毫秒）
 * @retval void
 * @details 通过循环调用delay_us(1000)实现1毫秒的延时效果，
 *          每次循环延时1000微秒，累计达到nms毫秒的总延时
 * @note 此函数为阻塞延时，在延时期间CPU无法处理其他任务
 */
void delay_ms(u32 nms)
{
    /* 每次外层循环延时1毫秒（1000微秒） */
    while(nms--)
    {
        delay_us(1000);    /* 每次延时1000微秒，即1毫秒 */
    }
}
