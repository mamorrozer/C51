/*------------------------------------------------------------
 * 文件：timer.c
 * 作用：系统节拍源，提供 1ms 中断驱动并派生 100ms 游戏帧节拍。
 * 框架：
 *   - Timer0_Init_1ms() 完成定时器0配置
 *   - Timer0_Routine() 每 1ms 进入一次：重装计数、推进蜂鸣器、累加时基、置位100ms标志
 *-----------------------------------------------------------*/
#include <REGX52.H>
#include "timer.h"
#include "beep.h"

volatile bit g_tick_100ms = 0;
volatile unsigned long g_game_time_ms = 0;

void Timer0_Init_1ms(void)
{
    /* 11.0592MHz / 12T 下，1ms重装值约为 65536-921=0xFC67。 */
    TMOD &= 0xF0;
    TMOD |= 0x01;
    TH0 = 0xFC;
    TL0 = 0x67;
    ET0 = 1;
    EA = 1;
    TR0 = 1;
}

void Timer0_Routine(void) interrupt 1
{
    static unsigned char ms_cnt = 0;

    /* 重装 1ms 定时初值，保持固定节拍。 */
    TH0 = 0xFC;
    TL0 = 0x67;

    /* 全局毫秒时钟 + 声音状态机都依赖这个 1ms 中断。 */
    g_game_time_ms++;
    Beep_Tick();

    ms_cnt++;
    if (ms_cnt >= 100)
    {
        ms_cnt = 0;
        g_tick_100ms = 1;
    }
}
