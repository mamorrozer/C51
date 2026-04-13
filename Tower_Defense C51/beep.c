/*------------------------------------------------------------
 * 文件：beep.c
 * 作用：无源/有源蜂鸣器控制（按低电平响、高电平静音模型）。
 * 框架：
 *   1) Beep_Bip() 写入“响多久/响几次”请求；
 *   2) Beep_Tick() 每 1ms 推进一次状态机，处理响铃段与间隔段。
 *-----------------------------------------------------------*/
#include <REGX52.H>
#include "beep.h"

sbit BEEP_PIN = P3^7;

static unsigned int beep_keep_ms = 0;
static unsigned char beep_repeat = 0;
static unsigned int beep_gap_ms = 0;

void Beep_Init(void)
{
    /* 默认关闭蜂鸣器，避免上电误响。 */
    BEEP_PIN = 1;
}

void Beep_Bip(unsigned char times, unsigned int ms)
{
    if (times == 0 || ms == 0)
    {
        return;
    }
    beep_repeat = times;
    beep_keep_ms = ms;
    beep_gap_ms = 0;
    /* 立即进入第一段响铃。 */
    BEEP_PIN = 0;
}

void Beep_Tick(void)
{
    if (beep_repeat == 0)
    {
        /* 没有待处理蜂鸣请求时保持静音。 */
        BEEP_PIN = 1;
        return;
    }

    if (beep_keep_ms > 0)
    {
        beep_keep_ms--;
        if (beep_keep_ms == 0)
        {
            BEEP_PIN = 1;
            beep_gap_ms = 80;
            beep_repeat--;
        }
        return;
    }

    if (beep_gap_ms > 0)
    {
        beep_gap_ms--;
        if (beep_gap_ms == 0 && beep_repeat > 0)
        {
            beep_keep_ms = 60;
            BEEP_PIN = 0;
        }
    }
}
