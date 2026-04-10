#include <REGX52.H>
#include "delay.h"

void DelayMs(unsigned int ms)
{
    unsigned char i, j;
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
