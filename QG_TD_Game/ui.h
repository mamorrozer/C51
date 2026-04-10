#ifndef __UI_H__
#define __UI_H__

#include "game.h"

#define UI_START      0
#define UI_MODE       1
#define UI_PLAY       2
#define UI_PAUSE      3
#define UI_RESULT     4

extern unsigned char g_ui_state;
extern unsigned char g_mode_select;

void UI_Init(void);
void UI_DrawStart(void);
void UI_DrawMode(void);
void UI_DrawGame(void);
void UI_DrawPause(void);
void UI_DrawResult(void);
void UI_HandleKey(unsigned char key);

#endif
