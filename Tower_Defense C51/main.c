#include <REGX52.H>
#include "delay.h"
#include "keypad.h"
#include "lcd12864.h"
#include "timer.h"
#include "beep.h"
#include "game.h"
#include "ui.h"

static void RenderByState(void)
{
    if (g_ui_state == UI_START) UI_DrawStart();
    else if (g_ui_state == UI_MODE) UI_DrawMode();
    else if (g_ui_state == UI_PLAY) UI_DrawGame();
    else if (g_ui_state == UI_PAUSE) UI_DrawPause();
    else UI_DrawResult();
}

void main(void)
{
    unsigned char key;

    LCD_Init();
    Beep_Init();
    Timer0_Init_1ms();
    UI_Init();
    Game_LoadBestScore();

    while (1)
    {
        key = Keypad_GetKey();
        if (key != KEY_NONE)
        {
            UI_HandleKey(key);
            RenderByState();
        }

        if (g_tick_100ms)
        {
            g_tick_100ms = 0;
            if (g_ui_state == UI_PLAY)
            {
                Game_Update100ms();
                if (g_game.game_over != GAME_RUNNING)
                {
                    Game_SaveBestScoreIfNeed();
                    g_ui_state = UI_RESULT;
                }
            }
            RenderByState();
        }
    }
}
