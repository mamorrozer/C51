#include <REGX52.H>
#include "delay.h"
#include "keypad.h"
#include "oled.h"
#include "timer.h"
#include "beep.h"
#include "game.h"
#include "ui.h"

static void RenderByState(void)
{
    /* UI 状态机统一渲染入口：根据当前状态选择对应页面绘制函数。 */
    if (g_ui_state == UI_START) UI_DrawStart();
    else if (g_ui_state == UI_MODE) UI_DrawMode();
    else if (g_ui_state == UI_PLAY) UI_DrawGame();
    else if (g_ui_state == UI_PAUSE) UI_DrawPause();
    else UI_DrawResult();
}

void main(void)
{
    unsigned char key;

    /* 系统启动顺序：先初始化外设，再初始化 UI 与游戏数据。 */
    OLED_Init();
    Beep_Init();
    Timer0_Init_1ms();
    UI_Init();
    Game_LoadBestScore();

    while (1)
    {
        /* 轮询输入事件：有键值就交给 UI 状态机处理。 */
        key = Keypad_GetKey();
        if (key != KEY_NONE)
        {
            UI_HandleKey(key);
            RenderByState();
        }

        /* 100ms 节拍到来时推进游戏逻辑，并刷新界面。 */
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
