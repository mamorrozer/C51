#ifndef __LCD12864_H__
#define __LCD12864_H__

void LCD_Init(void);
void LCD_Clear(void);
void LCD_SetCursor(unsigned char row, unsigned char col);
void LCD_WriteChar(unsigned char ch);
void LCD_WriteString(unsigned char row, unsigned char col, char code *str);

#endif
