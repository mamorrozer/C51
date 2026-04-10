#include <REGX52.H>
#include "delay.h"

void DelayMs(unsigned int ms)
{
    unsigned char i, j;
    /* 该延时参数基于 11.0592MHz 晶振经验值校准。 */
    while (ms--)
    {
        i = 2;
        j = 239;
        do
        {
            while (--j);
        } while (--i);
    }
}
