#include <REGX52.H>
#include "keypad.h"
#include "delay.h"

#define WAIT_RELEASE(pin)                      \
    do                                         \
    {                                          \
        unsigned int timeout = 0;              \
        while ((pin) == 0 && timeout < 50000)  \
        {                                      \
            timeout++;                         \
        }                                      \
    } while (0)

unsigned char Keypad_GetKey(void)
{
    unsigned char key = 0;

    P1 = 0xFF;
    P1_3 = 0;
    if (P1_7 == 0) { DelayMs(20); WAIT_RELEASE(P1_7); DelayMs(20); key = 1;  }
    if (P1_6 == 0) { DelayMs(20); WAIT_RELEASE(P1_6); DelayMs(20); key = 5;  }
    if (P1_5 == 0) { DelayMs(20); WAIT_RELEASE(P1_5); DelayMs(20); key = 9;  }
    if (P1_4 == 0) { DelayMs(20); WAIT_RELEASE(P1_4); DelayMs(20); key = 13; }
    if (key) return key;

    P1 = 0xFF;
    P1_2 = 0;
    if (P1_7 == 0) { DelayMs(20); WAIT_RELEASE(P1_7); DelayMs(20); key = 2;  }
    if (P1_6 == 0) { DelayMs(20); WAIT_RELEASE(P1_6); DelayMs(20); key = 6;  }
    if (P1_5 == 0) { DelayMs(20); WAIT_RELEASE(P1_5); DelayMs(20); key = KEY_REMOVE; }
    if (P1_4 == 0) { DelayMs(20); WAIT_RELEASE(P1_4); DelayMs(20); key = 14; }
    if (key) return key;

    P1 = 0xFF;
    P1_1 = 0;
    if (P1_7 == 0) { DelayMs(20); WAIT_RELEASE(P1_7); DelayMs(20); key = 3;  }
    if (P1_6 == 0) { DelayMs(20); WAIT_RELEASE(P1_6); DelayMs(20); key = 7;  }
    if (P1_5 == 0) { DelayMs(20); WAIT_RELEASE(P1_5); DelayMs(20); key = 11; }
    if (P1_4 == 0) { DelayMs(20); WAIT_RELEASE(P1_4); DelayMs(20); key = 15; }
    if (key) return key;

    P1 = 0xFF;
    P1_0 = 0;
    if (P1_7 == 0) { DelayMs(20); WAIT_RELEASE(P1_7); DelayMs(20); key = 4;  }
    if (P1_6 == 0) { DelayMs(20); WAIT_RELEASE(P1_6); DelayMs(20); key = 8;  }
    if (P1_5 == 0) { DelayMs(20); WAIT_RELEASE(P1_5); DelayMs(20); key = 12; }
    if (P1_4 == 0) { DelayMs(20); WAIT_RELEASE(P1_4); DelayMs(20); key = 16; }

    return key;
}
