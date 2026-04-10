#include <REGX52.H>
#include "timer.h"
#include "beep.h"

volatile bit g_tick_100ms = 0;
volatile unsigned long g_game_time_ms = 0;

void Timer0_Init_1ms(void)
{
    TMOD &= 0xF0;
    TMOD |= 0x01;
    TH0 = 0xFC;
    TL0 = 0x18;
    ET0 = 1;
    EA = 1;
    TR0 = 1;
}

void Timer0_Routine(void) interrupt 1
{
    static unsigned char ms_cnt = 0;

    TH0 = 0xFC;
    TL0 = 0x18;

    g_game_time_ms++;
    Beep_Tick();

    ms_cnt++;
    if (ms_cnt >= 100)
    {
        ms_cnt = 0;
        g_tick_100ms = 1;
    }
}
