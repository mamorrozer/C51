/*------------------------------------------------------------
 * 文件：timer.h
 * 作用：提供 1ms 系统节拍与 100ms 游戏节拍标志。
 * 框架：
 *   - Timer0_Init_1ms() 配置定时器0中断
 *   - g_tick_100ms      主循环读取的 100ms 事件标志
 *   - g_game_time_ms    运行时毫秒计数
 *-----------------------------------------------------------*/
#ifndef __TIMER_H__
#define __TIMER_H__

extern volatile bit g_tick_100ms;
extern volatile unsigned long g_game_time_ms;

/* 初始化定时器0为 1ms 周期中断。 */
void Timer0_Init_1ms(void);

#endif
