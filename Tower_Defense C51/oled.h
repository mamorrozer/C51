#ifndef __OLED_H__
#define __OLED_H__

/* OLED 驱动对外接口：按“4行×16字符”文本方式使用，内部映射到 SSD1306 像素页。 */
void OLED_Init(void);
void OLED_Clear(void);
void OLED_SetCursor(unsigned char row, unsigned char col);
void OLED_WriteChar(unsigned char ch);
void OLED_WriteString(unsigned char row, unsigned char col, char code *str);

#endif
