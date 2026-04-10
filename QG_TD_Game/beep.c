#include <REGX52.H>
#include "beep.h"

sbit BEEP_PIN = P3^7;

static unsigned int beep_keep_ms = 0;
static unsigned char beep_repeat = 0;
static unsigned int beep_gap_ms = 0;

void Beep_Init(void)
{
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
    BEEP_PIN = 0;
}

void Beep_Tick(void)
{
    if (beep_repeat == 0)
    {
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
