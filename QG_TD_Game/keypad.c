#include <REGX52.H>
#include "keypad.h"
#include "delay.h"

unsigned char Keypad_GetKey(void)
{
    unsigned char key = 0;

    P1 = 0xFF;
    P1_3 = 0;
    if (P1_7 == 0) { DelayMs(20); while (P1_7 == 0); DelayMs(20); key = 1;  }
    if (P1_6 == 0) { DelayMs(20); while (P1_6 == 0); DelayMs(20); key = 5;  }
    if (P1_5 == 0) { DelayMs(20); while (P1_5 == 0); DelayMs(20); key = 9;  }
    if (P1_4 == 0) { DelayMs(20); while (P1_4 == 0); DelayMs(20); key = 13; }

    P1 = 0xFF;
    P1_2 = 0;
    if (P1_7 == 0) { DelayMs(20); while (P1_7 == 0); DelayMs(20); key = 2;  }
    if (P1_6 == 0) { DelayMs(20); while (P1_6 == 0); DelayMs(20); key = 6;  }
    if (P1_5 == 0) { DelayMs(20); while (P1_5 == 0); DelayMs(20); key = 10; }
    if (P1_4 == 0) { DelayMs(20); while (P1_4 == 0); DelayMs(20); key = 14; }

    P1 = 0xFF;
    P1_1 = 0;
    if (P1_7 == 0) { DelayMs(20); while (P1_7 == 0); DelayMs(20); key = 3;  }
    if (P1_6 == 0) { DelayMs(20); while (P1_6 == 0); DelayMs(20); key = 7;  }
    if (P1_5 == 0) { DelayMs(20); while (P1_5 == 0); DelayMs(20); key = 11; }
    if (P1_4 == 0) { DelayMs(20); while (P1_4 == 0); DelayMs(20); key = 15; }

    P1 = 0xFF;
    P1_0 = 0;
    if (P1_7 == 0) { DelayMs(20); while (P1_7 == 0); DelayMs(20); key = 4;  }
    if (P1_6 == 0) { DelayMs(20); while (P1_6 == 0); DelayMs(20); key = 8;  }
    if (P1_5 == 0) { DelayMs(20); while (P1_5 == 0); DelayMs(20); key = 12; }
    if (P1_4 == 0) { DelayMs(20); while (P1_4 == 0); DelayMs(20); key = 16; }

    return key;
}
