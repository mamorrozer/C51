/*------------------------------------------------------------
 * 文件：main.c
 * 作用：程序入口，负责模块初始化与主循环调度。
 * 框架：
 *   - 初始化：显示/蜂鸣器/定时器/UI/存档
 *   - 主循环：轮询输入事件 + 消费 100ms 逻辑节拍
 *   - 渲染：每次状态变化后统一走 RenderByState()
 *-----------------------------------------------------------*/
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

    /* 启动阶段：
     * 1) OLED/蜂鸣器/定时器等硬件就绪
     * 2) UI 状态机复位到开始页
     * 3) 从 EEPROM 读出历史最高分（掉电保留）
     */
    OLED_Init();
    Beep_Init();
    Timer0_Init_1ms();
    UI_Init();
    Game_LoadBestScore();

    while (1)
    {
        /* 主循环采用“事件驱动 + 固定步长”混合模式：
         * - 按键是事件驱动（有键就立刻处理）
         * - 战斗逻辑是固定 100ms 步长（保证各难度节奏稳定）
         */
        key = Keypad_GetKey();
        if (key != KEY_NONE)
        {
            UI_HandleKey(key);
            RenderByState();
        }

        /* 100ms 帧：只在 UI_PLAY 才推进战斗，其它页面只做显示刷新。 */
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
