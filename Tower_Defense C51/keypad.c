/*------------------------------------------------------------
 * 文件：keypad.c
 * 作用：输入驱动层，将 4x4 矩阵键盘转换为统一 KEY_* 键值。
 * 框架：
 *   1) 扫描行列得到原始键号（1~16）；
 *   2) 把原始键号映射成游戏键值；
 *   3) 使用“按下锁存、松开解锁”避免长按连发。
 *-----------------------------------------------------------*/
#include <REGX52.H>
#include "keypad.h"

/* 开发板矩阵键盘连接：
 * 行：P1.7~P1.4
 * 列：P1.3~P1.0
 */
sbit KP_ROW1 = P1^7;
sbit KP_ROW2 = P1^6;
sbit KP_ROW3 = P1^5;
sbit KP_ROW4 = P1^4;
sbit KP_COL1 = P1^3;
sbit KP_COL2 = P1^2;
sbit KP_COL3 = P1^1;
sbit KP_COL4 = P1^0;

static void Matrix_SetIdle(void)
{
    KP_ROW1 = 1;
    KP_ROW2 = 1;
    KP_ROW3 = 1;
    KP_ROW4 = 1;
    KP_COL1 = 1;
    KP_COL2 = 1;
    KP_COL3 = 1;
    KP_COL4 = 1;
}

static unsigned char Matrix_ReadColIndex(void)
{
    if (KP_COL1 == 0) return 1;
    if (KP_COL2 == 0) return 2;
    if (KP_COL3 == 0) return 3;
    if (KP_COL4 == 0) return 4;
    return 0;
}

static unsigned char Matrix_ReadRawKey(void)
{
    unsigned char col;

    Matrix_SetIdle();

    KP_ROW1 = 0;
    col = Matrix_ReadColIndex();
    if (col != 0)
    {
        Matrix_SetIdle();
        return col;
    }
    KP_ROW1 = 1;

    KP_ROW2 = 0;
    col = Matrix_ReadColIndex();
    if (col != 0)
    {
        Matrix_SetIdle();
        return (unsigned char)(4 + col);
    }
    KP_ROW2 = 1;

    KP_ROW3 = 0;
    col = Matrix_ReadColIndex();
    if (col != 0)
    {
        Matrix_SetIdle();
        return (unsigned char)(8 + col);
    }
    KP_ROW3 = 1;

    KP_ROW4 = 0;
    col = Matrix_ReadColIndex();
    if (col != 0)
    {
        Matrix_SetIdle();
        return (unsigned char)(12 + col);
    }

    Matrix_SetIdle();
    return 0;
}

static unsigned char MapRawKeyToGameKey(unsigned char raw_key)
{
    /* 默认键位：
     * 6=上，14=下，9=左，11=右
     * 1/2/3 = 快捷工具切换（射手/墙/铲），10=确认/执行
     * 13=暂停，16=返回
     */
    switch (raw_key)
    {
    case 6:  return KEY_UP;
    case 14: return KEY_DOWN;
    case 9:  return KEY_LEFT;
    case 11: return KEY_RIGHT;
    case 10: return KEY_SHOOTER;

    case 1:  return KEY_TOOL_SHOOTER;
    case 2:  return KEY_TOOL_WALL;
    case 3:  return KEY_TOOL_REMOVE;

    case 13: return KEY_PAUSE;
    case 16: return KEY_BACK;
    default: return KEY_NONE;
    }
}

unsigned char Keypad_GetKey(void)
{
    static bit key_lock = 0;
    unsigned char raw_key;
    unsigned char key;

    raw_key = Matrix_ReadRawKey();

    if (key_lock)
    {
        if (raw_key == 0)
        {
            key_lock = 0;
        }
        return KEY_NONE;
    }

    if (raw_key == 0)
    {
        return KEY_NONE;
    }

    key = MapRawKeyToGameKey(raw_key);
    if (key != KEY_NONE)
    {
        key_lock = 1;
    }
    return key;
}
