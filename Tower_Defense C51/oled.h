/*------------------------------------------------------------
 * 文件：oled.h
 * 作用：SSD1306 OLED 的文本化显示接口。
 * 框架：对外暴露“4行×16列字符屏”抽象，内部由 oled.c 完成像素映射。
 *-----------------------------------------------------------*/
#ifndef __OLED_H__
#define __OLED_H__

/* OLED 驱动对外接口：按“4行×16字符”文本方式使用，内部映射到 SSD1306 像素页。 */
/* 初始化 OLED 硬件并清屏。 */
void OLED_Init(void);
/* 清空显示内容并将光标归位。 */
void OLED_Clear(void);
/* 设置文本光标（row:0~3, col:0~15）。 */
void OLED_SetCursor(unsigned char row, unsigned char col);
/* 在当前光标写一个字符并自动右移。 */
void OLED_WriteChar(unsigned char ch);
/* 从指定位置连续写字符串。 */
void OLED_WriteString(unsigned char row, unsigned char col, char code *str);

#endif
