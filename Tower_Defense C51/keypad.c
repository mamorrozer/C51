#include <REGX52.H>
#include "keypad.h"
#include "i2c.h"

/* HW504 摇杆改为 PCF8591 采样（I2C 设备）：
 * PCF8591 A0/A1/A2 建议接 GND，地址为 0x90(写)/0x91(读)；
 * CH0 接摇杆 X，CH1 接摇杆 Y；
 * 摇杆按压键 SW 仍接 P3.3（低电平按下）。
 */
sbit JOY_SW = P3^3;

#define PCF8591_ADDR_WRITE     0x90
#define PCF8591_ADDR_READ      0x91

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

static unsigned char PCF8591_Read(unsigned char channel)
{
    unsigned char control;
    unsigned char value;

    /* 控制字：0x40 选择单端输入模式，低两位为通道号(0~3)。 */
    control = (unsigned char)(0x40 | (channel & 0x03));

    I2C_Start();
    I2C_Write(PCF8591_ADDR_WRITE);
    if (I2C_ReadAck()) goto stop_error;
    I2C_Write(control);
    if (I2C_ReadAck()) goto stop_error;

    /* 重新起始切到读，PCF8591 首字节是“上一拍缓存”，需要先读掉。 */
    I2C_Start();
    I2C_Write(PCF8591_ADDR_READ);
    if (I2C_ReadAck()) goto stop_error;
    I2C_Read(); /* 首字节为无效缓存值，必须先读掉。 */
    I2C_SendAck(0);

    /* 第二字节才是当前通道有效值，读完发送 NACK 结束。 */
    value = I2C_Read();
    I2C_SendAck(1);
    I2C_Stop();
    return value;

stop_error:
    /* I2C 失败时返回中心值，避免误判成极限方向。 */
    I2C_Stop();
    return ADC_CENTER;
}

static unsigned char AbsDiff(unsigned char a, unsigned char b)
{
    return (a > b) ? (a - b) : (b - a);
}

static JoyDir HW504_GetDirection(unsigned char x, unsigned char y)
{
    unsigned char dx = AbsDiff(x, ADC_CENTER);
    unsigned char dy = AbsDiff(y, ADC_CENTER);

    /* X/Y 都在死区内时判定为中立，抑制轻微抖动。 */
    if (dx < ADC_DEAD_ZONE && dy < ADC_DEAD_ZONE)
    {
        return JOY_DIR_CENTER;
    }

    /* 谁偏离中心更大就按哪个轴判方向，减少斜向误触。 */
    if (dx >= dy)
    {
        if (x > (ADC_CENTER + ADC_DIR_STRONG_DELTA)) return JOY_DIR_RIGHT;
        if (x < (ADC_CENTER - ADC_DIR_STRONG_DELTA)) return JOY_DIR_LEFT;
    }
    else
    {
        /* HW504 常见接法：Y 增大为上移；若“上推摇杆但光标下移”，互换这两行返回值 */
        if (y > (ADC_CENTER + ADC_DIR_STRONG_DELTA)) return JOY_DIR_UP;
        if (y < (ADC_CENTER - ADC_DIR_STRONG_DELTA)) return JOY_DIR_DOWN;
    }

    return JOY_DIR_CENTER;
}

unsigned char Keypad_GetKey(void)
{
    /* repeat_lock：方向/按压未回中前只触发一次，避免长按连发。 */
    static bit repeat_lock = 0;
    /* 降采样：不是每次轮询都读 ADC，减轻 I2C 负担并平滑输入。 */
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
        x_cache = PCF8591_Read(ADC_CH_X);
        y_cache = PCF8591_Read(ADC_CH_Y);
    }
    x = x_cache;
    y = y_cache;
    dir = HW504_GetDirection(x, y);
    pressed = (JOY_SW == 0) ? 1 : 0;

    if (repeat_lock)
    {
        /* 摇杆回中且按压松开后解锁，允许下一次按键事件。 */
        if (dir == JOY_DIR_CENTER && !pressed)
        {
            repeat_lock = 0;
        }
        return KEY_NONE;
    }

    if (pressed)
    {
        /* 按下摇杆时：方向键映射到功能键（建造/暂停/返回等）。 */
        if (dir == JOY_DIR_UP) key = KEY_WALL;
        else if (dir == JOY_DIR_DOWN) key = KEY_REMOVE;
        else if (dir == JOY_DIR_LEFT) key = KEY_PAUSE;
        else if (dir == JOY_DIR_RIGHT) key = KEY_BACK;
        else key = KEY_SHOOTER;
    }
    else
    {
        /* 未按下摇杆时：方向键映射为纯移动。 */
        if (dir == JOY_DIR_UP) key = KEY_UP;
        else if (dir == JOY_DIR_DOWN) key = KEY_DOWN;
        else if (dir == JOY_DIR_LEFT) key = KEY_LEFT;
        else if (dir == JOY_DIR_RIGHT) key = KEY_RIGHT;
    }

    if (key != KEY_NONE)
    {
        /* 一旦触发有效按键立即加锁，等待下一次“回中+松开”。 */
        repeat_lock = 1;
    }

    return key;
}
