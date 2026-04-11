#include <REGX52.H>
#include "lcd12864.h"
#include "delay.h"

sbit LCD_RS = P2^3;
sbit LCD_RW = P2^4;
sbit LCD_EN = P2^2;

static void LCD_BusyWait(void)
{
    DelayMs(2);
}

static void LCD_WriteCmd(unsigned char cmd)
{
    LCD_BusyWait();
    LCD_RS = 0;
    LCD_RW = 0;
    P0 = cmd;
    LCD_EN = 1;
    LCD_EN = 0;
}

void LCD_WriteChar(unsigned char ch)
{
    LCD_BusyWait();
    LCD_RS = 1;
    LCD_RW = 0;
    P0 = ch;
    LCD_EN = 1;
    LCD_EN = 0;
}

void LCD_Clear(void)
{
    LCD_WriteCmd(0x01);
    DelayMs(2);
}

void LCD_SetCursor(unsigned char row, unsigned char col)
{
    unsigned char addr;
    if (row == 0) addr = 0x80 + col;
    else if (row == 1) addr = 0x90 + col;
    else if (row == 2) addr = 0x88 + col;
    else addr = 0x98 + col;
    LCD_WriteCmd(addr);
}

void LCD_WriteString(unsigned char row, unsigned char col, char code *str)
{
    LCD_SetCursor(row, col);
    while (*str)
    {
        LCD_WriteChar(*str++);
    }
}

void LCD_Init(void)
{
    LCD_WriteCmd(0x30);
    LCD_WriteCmd(0x0C);
    LCD_WriteCmd(0x06);
    LCD_Clear();
}
