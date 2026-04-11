#ifndef __TIMER_H__
#define __TIMER_H__

extern volatile bit g_tick_100ms;
extern volatile unsigned long g_game_time_ms;

void Timer0_Init_1ms(void);

#endif
