/*------------------------------------------------------------
 * 文件：ui.c
 * 作用：UI 状态机与界面绘制，实现“开始/选关/游戏/暂停/结算”页面。
 * 框架：
 *   - 维护双缓冲：screen_now / screen_old
 *   - 页面函数负责填充 screen_now
 *   - FlushDiff() 只把变化字符写到 OLED，降低闪烁
 * 还有一些函数说明在.h文件注释
 *-----------------------------------------------------------*/
#include "ui.h"
#include "oled.h"
#include "keypad.h"

unsigned char g_ui_state = UI_START;
unsigned char g_mode_select = 0;
static unsigned char g_special_select = 0;


/* 【内存布局】
将大缓冲放到 IDATA，避免挤占 128B DATA 直寻址空间。
把 64 字节的文本缓冲强制放到 idata，虽然比 data 慢一个周期，
但游戏画面 100ms 才刷一次，性能损失不可见，换来 data 区的绝对安全。
*/
static unsigned char idata screen_now[4][16];
static unsigned char idata screen_old[4][16];

static void FillLine(unsigned char row, unsigned char ch)
{
    unsigned char i;
    /* 用指定字符覆盖整行，常用于先清空再写文本。 */
    for (i = 0; i < 16; i++) screen_now[row][i] = ch;
}

static void ClearScreen(void)
{
    unsigned char r;
    for (r = 0; r < 4; r++) FillLine(r, ' ');
}

static void DrawLaneToRow(unsigned char lane, unsigned char row)
{
    unsigned char i;
    unsigned char lane_chars[MAP_COLS];
    Game_BuildLaneChars(lane, (char *)lane_chars);
    for (i = 0; i < MAP_COLS; i++)
    {
        screen_now[row][i] = (lane_chars[i] == 0 || lane_chars[i] == ' ') ? '-' : lane_chars[i];
    }
}

static void PutText(unsigned char row, unsigned char col, char code *str)
{
    /* 按列连续写字符串，超出 16 列会自动截断。 */
    while (*str && col < 16)
    {
        screen_now[row][col++] = (unsigned char)(*str++);
    }
}

static void PutNum2(unsigned char row, unsigned char col, unsigned int num)
{
    /* 以两位十进制显示（00~99），用于资源/生命等短数字。 */
    if (col > 14) return;
    screen_now[row][col] = (char)('0' + (num / 10) % 10);
    screen_now[row][col + 1] = (char)('0' + num % 10);
}

static void PutNum4(unsigned char row, unsigned char col, unsigned int num)
{
    /* 以四位十进制显示（0000~9999），用于分数/时长。 */
    if (col > 12) return;
    screen_now[row][col] = (char)('0' + (num / 1000) % 10);
    screen_now[row][col + 1] = (char)('0' + (num / 100) % 10);
    screen_now[row][col + 2] = (char)('0' + (num / 10) % 10);
    screen_now[row][col + 3] = (char)('0' + num % 10);
}

static void WriteCellAndCommit(unsigned char row, unsigned char col, unsigned char ch)
{
    OLED_SetCursor(row, col);
    if (OLED_WriteCharChecked(ch))
    {
        screen_old[row][col] = ch;
    }
}

/* 【硬件妥协】OLED 接在 P0（开漏），板载 10K 上拉。
在高负载下直接全刷容易掉数据/花屏。
增量刷新只发变化字符，把 I2C 总线负载压到极低，
从而在‘不改硬件、不飞线’前提下保证通信稳定。
*/
static void FlushDiff(void)
{
    unsigned char r, c;
    /* 游戏页与开始页都采用整行重绘，避免后半段残影/写漏。 */
    if (g_ui_state == UI_PLAY || g_ui_state == UI_START)
    {
        for (c = 0; c < 16; c++)
        {
            if (screen_now[0][c] != screen_old[0][c])
            {
                WriteCellAndCommit(0, c, screen_now[0][c]);
            }
        }

        for (r = 1; r < 4; r++)
        {
            for (c = 0; c < 16; c++)
            {
                WriteCellAndCommit(r, c, screen_now[r][c]);
            }
        }
        return;
    }

    /* 其余页面走局部刷新，减少闪烁。 */
    for (r = 0; r < 4; r++)
    {
        for (c = 0; c < 16; c++)
        {
            if (screen_now[r][c] != screen_old[r][c])
            {
                WriteCellAndCommit(r, c, screen_now[r][c]);
            }
        }
    }
}

void UI_Init(void)
{
    unsigned char r, c;
    /* UI 初始状态：开机进入开始页，模式默认为 EASY。 */
    g_ui_state = UI_START;
    g_mode_select = 0;
    g_special_select = 0;
    for (r = 0; r < 4; r++)
        for (c = 0; c < 16; c++)
        {
            screen_now[r][c] = ' ';
            screen_old[r][c] = ' ';
        }
}

void UI_DrawStart(void)
{
    /* 开始页只负责“特殊模式开关 + 进入下一页”。 */
    ClearScreen();
    PutText(0, 0, "TOWER DEFENSE");
    PutText(1, 0, (g_special_select == 0) ? "SPECIAL:OFF" : "SPECIAL:ON");
    PutText(2, 0, "6/14:TOGGLE");
    PutText(3, 0, "10:NEXT");
    FlushDiff();
}

void UI_DrawMode(void)
{
    unsigned char mode_id = g_mode_select ? 2 : 0;
    /* 难度页不改任何战斗数据，只记录选择索引。 */
    ClearScreen();

    PutText(0, 0, "MODE");
    PutText(0, 9, "HI");
    PutNum4(0, 12, Game_GetBestScoreByMode(mode_id) % 10000);
    PutText(1, 0, (g_mode_select == 0) ? ">EASY" : " EASY");
    PutText(2, 0, (g_mode_select == 1) ? ">HARD" : " HARD");
    FlushDiff();
}

void UI_DrawGame(void)
{
    ClearScreen();

    /* 顶栏详细显示：工具/生命/资源/分数。 */
    PutText(0, 0, "T");
    if (g_game.selected_tool == TOOL_SHOOTER) screen_now[0][1] = 'S';
    else if (g_game.selected_tool == TOOL_WALL) screen_now[0][1] = 'W';
    else screen_now[0][1] = 'R';
    PutText(0, 3, "H");
    PutNum2(0, 4, g_game.life);
    PutText(0, 7, "R");
    PutNum2(0, 8, g_game.resource % 100);
    PutText(0, 11, "S");
    PutNum4(0, 12, g_game.score % 10000);

    /* 轨道文字由 Game_BuildLaneChars 统一生成，UI 只做复制显示。 */
    DrawLaneToRow(0, 1);
    DrawLaneToRow(1, 2);
    DrawLaneToRow(2, 3);

    if (g_game.special_mode && g_game.fog_cols > 0)
    {
        unsigned char i;
        unsigned char fog_begin = (unsigned char)(MAP_COLS - g_game.fog_cols);
        for (i = fog_begin; i < MAP_COLS; i++)
        {
            screen_now[1][i] = '?';
            screen_now[2][i] = '?';
            screen_now[3][i] = '?';
        }
    }

    FlushDiff();
}

void UI_DrawPause(void)
{
    /* 暂停页是纯提示页，不推进战斗。 */
    ClearScreen();
    PutText(0, 0, "PAUSE");
    PutText(1, 0, "13:CONT");
    PutText(2, 0, "16:RES");
    FlushDiff();
}

void UI_DrawResult(void)
{
    /* 结算数据都来自 game.c 的当前状态快照。 */
    ClearScreen();

    if (g_game.game_over == GAME_WIN) PutText(0, 0, "WIN");
    else PutText(0, 0, "LOSE");
    PutText(1, 0, "Tm:");
    PutNum4(1, 3, Game_GetSurviveSecond() % 10000);
    PutText(2, 0, "Sc:");
    PutNum4(2, 3, g_game.score % 10000);
    PutText(3, 0, "Hi:");
    PutNum4(3, 3, Game_GetBestScore() % 10000);
    FlushDiff();
}

void UI_HandleKey(unsigned char key)
{
    /* UI 状态机核心：同一个键会被解释成不同动作。 */
    if (g_ui_state == UI_START)
    {
        if (key == KEY_UP || key == KEY_DOWN)
        {
            g_special_select = g_special_select ? 0 : 1;
            return;
        }
        if (key == KEY_SHOOTER)
        {
            Game_SetSpecialMode(g_special_select);
            g_ui_state = UI_MODE;
        }
        return;
    }

    if (g_ui_state == UI_MODE)
    {
        if (key == KEY_UP || key == KEY_DOWN) g_mode_select = g_mode_select ? 0 : 1;
        if (key == KEY_SHOOTER)
        {
            Game_Init(g_mode_select ? 2 : 0);
            g_ui_state = UI_PLAY;
        }
        if (key == KEY_BACK)
        {
            g_ui_state = UI_START;
        }
        return;
    }

    if (g_ui_state == UI_PLAY)
    {
        if (key == KEY_PAUSE)
        {
            g_ui_state = UI_PAUSE;
            return;
        }
        if (key == KEY_BACK)
        {
            g_game.game_over = GAME_LOSE;
            Game_SaveBestScoreIfNeed();
            g_ui_state = UI_RESULT;
            return;
        }
        Game_HandleKey(key);
        if (g_game.game_over != GAME_RUNNING)
        {
            Game_SaveBestScoreIfNeed();
            g_ui_state = UI_RESULT;
        }
        return;
    }

    if (g_ui_state == UI_PAUSE)
    {
        if (key == KEY_PAUSE)
        {
            g_ui_state = UI_PLAY;
            return;
        }
        if (key == KEY_BACK)
        {
            Game_SaveBestScoreIfNeed();
            g_ui_state = UI_RESULT;
        }
        return;
    }

    if (g_ui_state == UI_RESULT)
    {
        if (key == KEY_SHOOTER || key == KEY_BACK)
        {
            g_ui_state = UI_START;
        }
    }
}
