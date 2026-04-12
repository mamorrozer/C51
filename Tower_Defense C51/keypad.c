#include <REGX52.H>
#include <INTRINS.H>
#include "keypad.h"

/* ADC0832 连接（可按需改线）
 * CS  -> P3.0
 * CLK -> P3.1
 * DI  -> P3.2
 * DO  -> P3.4
 * HW504 按压键 SW -> P3.3（低电平按下）
 */
sbit ADC_CS   = P3^0;
sbit ADC_CLK  = P3^1;
sbit ADC_DI   = P3^2;
sbit ADC_DO   = P3^4;
sbit JOY_SW   = P3^3;

#define ADC_CH_X               0
#define ADC_CH_Y               1

#define ADC_CENTER             128
#define ADC_DEAD_ZONE          32
#define ADC_DIR_STRONG_DELTA   64
#define ADC_SAMPLE_DIV         2

typedef enum
{
    JOY_DIR_CENTER = 0,
    JOY_DIR_UP,
    JOY_DIR_DOWN,
    JOY_DIR_LEFT,
    JOY_DIR_RIGHT
} JoyDir;

static void ADC_Delay(void)
{
    _nop_();
    _nop_();
}

static unsigned char ADC0832_Read(unsigned char channel)
{
    unsigned char i;
    unsigned char dat = 0;

    ADC_CS = 1;
    ADC_CLK = 0;
    ADC_CS = 0;

    /* start */
    ADC_DI = 1;
    ADC_CLK = 1; ADC_Delay(); ADC_CLK = 0;
    /* single-ended */
    ADC_DI = 1;
    ADC_CLK = 1; ADC_Delay(); ADC_CLK = 0;
    /* channel select */
    ADC_DI = channel ? 1 : 0;
    ADC_CLK = 1; ADC_Delay(); ADC_CLK = 0;
    /* MSBF */
    ADC_DI = 1;
    ADC_CLK = 1; ADC_Delay(); ADC_CLK = 0;

    for (i = 0; i < 8; i++)
    {
        dat <<= 1;
        ADC_CLK = 1;
        ADC_Delay();
        if (ADC_DO) dat |= 0x01;
        ADC_CLK = 0;
    }

    ADC_CS = 1;
    return dat;
}

static unsigned char AbsDiff(unsigned char a, unsigned char b)
{
    return (a > b) ? (a - b) : (b - a);
}

static JoyDir HW504_GetDirection(unsigned char x, unsigned char y)
{
    unsigned char dx = AbsDiff(x, ADC_CENTER);
    unsigned char dy = AbsDiff(y, ADC_CENTER);

    if (dx < ADC_DEAD_ZONE && dy < ADC_DEAD_ZONE)
    {
        return JOY_DIR_CENTER;
    }

    if (dx >= dy)
    {
        if (x > (ADC_CENTER + ADC_DIR_STRONG_DELTA)) return JOY_DIR_RIGHT;
        if (x < (ADC_CENTER - ADC_DIR_STRONG_DELTA)) return JOY_DIR_LEFT;
    }
    else
    {
        /* HW504 常见接法：Y 增大为上移，若相反可互换这两行返回值 */
        if (y > (ADC_CENTER + ADC_DIR_STRONG_DELTA)) return JOY_DIR_UP;
        if (y < (ADC_CENTER - ADC_DIR_STRONG_DELTA)) return JOY_DIR_DOWN;
    }

    return JOY_DIR_CENTER;
}

unsigned char Keypad_GetKey(void)
{
    static bit hold_lock = 0;
    static unsigned char sample_div_cnt = 0;
    static unsigned char x_cache = ADC_CENTER;
    static unsigned char y_cache = ADC_CENTER;
    JoyDir dir;
    unsigned char key = KEY_NONE;
    bit pressed;
    unsigned char x;
    unsigned char y;

    sample_div_cnt++;
    if (sample_div_cnt >= ADC_SAMPLE_DIV)
    {
        sample_div_cnt = 0;
        x_cache = ADC0832_Read(ADC_CH_X);
        y_cache = ADC0832_Read(ADC_CH_Y);
    }
    x = x_cache;
    y = y_cache;
    dir = HW504_GetDirection(x, y);
    pressed = (JOY_SW == 0) ? 1 : 0;

    if (hold_lock)
    {
        if (dir == JOY_DIR_CENTER && !pressed)
        {
            hold_lock = 0;
        }
        return KEY_NONE;
    }

    if (pressed)
    {
        if (dir == JOY_DIR_UP) key = KEY_WALL;
        else if (dir == JOY_DIR_DOWN) key = KEY_REMOVE;
        else if (dir == JOY_DIR_LEFT) key = KEY_PAUSE;
        else if (dir == JOY_DIR_RIGHT) key = KEY_BACK;
        else key = KEY_SHOOTER;
    }
    else
    {
        if (dir == JOY_DIR_UP) key = KEY_UP;
        else if (dir == JOY_DIR_DOWN) key = KEY_DOWN;
        else if (dir == JOY_DIR_LEFT) key = KEY_LEFT;
        else if (dir == JOY_DIR_RIGHT) key = KEY_RIGHT;
    }

    if (key != KEY_NONE)
    {
        hold_lock = 1;
    }

    return key;
}
