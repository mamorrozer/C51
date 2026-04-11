#include "ui.h"
#include "lcd12864.h"
#include "keypad.h"
#include "beep.h"

unsigned char g_ui_state = UI_START;
unsigned char g_mode_select = 0;

static char screen_now[4][16];
static char screen_old[4][16];

static void FillLine(unsigned char row, char ch)
{
    unsigned char i;
    for (i = 0; i < 16; i++) screen_now[row][i] = ch;
}

static void PutText(unsigned char row, unsigned char col, char code *str)
{
    while (*str && col < 16)
    {
        screen_now[row][col++] = *str++;
    }
}

static void PutNum2(unsigned char row, unsigned char col, unsigned int num)
{
    if (col > 14) return;
    screen_now[row][col] = (char)('0' + (num / 10) % 10);
    screen_now[row][col + 1] = (char)('0' + num % 10);
}

static void PutNum4(unsigned char row, unsigned char col, unsigned int num)
{
    if (col > 12) return;
    screen_now[row][col] = (char)('0' + (num / 1000) % 10);
    screen_now[row][col + 1] = (char)('0' + (num / 100) % 10);
    screen_now[row][col + 2] = (char)('0' + (num / 10) % 10);
    screen_now[row][col + 3] = (char)('0' + num % 10);
}

static void FlushDiff(void)
{
    unsigned char r, c;
    for (r = 0; r < 4; r++)
    {
        for (c = 0; c < 16; c++)
        {
            if (screen_now[r][c] != screen_old[r][c])
            {
                LCD_SetCursor(r, c);
                LCD_WriteChar(screen_now[r][c]);
                screen_old[r][c] = screen_now[r][c];
            }
        }
    }
}

void UI_Init(void)
{
    unsigned char r, c;
    g_ui_state = UI_START;
    g_mode_select = 0;
    for (r = 0; r < 4; r++)
        for (c = 0; c < 16; c++)
            screen_old[r][c] = 0;
}

void UI_DrawStart(void)
{
    FillLine(0, ' ');
    FillLine(1, ' ');
    FillLine(2, ' ');
    FillLine(3, ' ');

    PutText(0, 0, "QG 51 TD GAME");
    PutText(1, 0, "2/8 move 5S 9W");
    PutText(2, 0, "13 pause 16 back");
    PutText(3, 0, "press 5 to start");
    FlushDiff();
}

void UI_DrawMode(void)
{
    FillLine(0, ' ');
    FillLine(1, ' ');
    FillLine(2, ' ');
    FillLine(3, ' ');

    PutText(0, 0, "MODE SELECT");
    PutText(1, 0, (g_mode_select == 0) ? ">EASY" : " EASY");
    PutText(2, 0, (g_mode_select == 1) ? ">MID " : " MID ");
    PutText(3, 0, (g_mode_select == 2) ? ">HARD" : " HARD");
    FlushDiff();
}

void UI_DrawGame(void)
{
    char lane_chars[MAP_COLS];
    unsigned char i;

    FillLine(0, ' ');
    FillLine(1, ' ');
    FillLine(2, ' ');
    FillLine(3, ' ');

    PutText(0, 0, "R");
    PutNum2(0, 1, g_game.resource % 100);
    PutText(0, 4, "H");
    PutNum2(0, 5, g_game.life);
    PutText(0, 8, "S");
    PutNum4(0, 9, g_game.score % 10000);

    Game_BuildLaneChars(0, lane_chars);
    screen_now[1][0] = '1'; screen_now[1][1] = ':';
    for (i = 0; i < MAP_COLS; i++) screen_now[1][2 + i] = lane_chars[i];

    Game_BuildLaneChars(1, lane_chars);
    screen_now[2][0] = '2'; screen_now[2][1] = ':';
    for (i = 0; i < MAP_COLS; i++) screen_now[2][2 + i] = lane_chars[i];

    Game_BuildLaneChars(2, lane_chars);
    screen_now[3][0] = '3'; screen_now[3][1] = ':';
    for (i = 0; i < MAP_COLS; i++) screen_now[3][2 + i] = lane_chars[i];

    FlushDiff();
}

void UI_DrawPause(void)
{
    FillLine(0, ' ');
    FillLine(1, ' ');
    FillLine(2, ' ');
    FillLine(3, ' ');
    PutText(0, 0, "GAME PAUSED");
    PutText(1, 0, "13:continue");
    PutText(2, 0, "16:result");
    PutText(3, 0, "5/9 disable");
    FlushDiff();
}

void UI_DrawResult(void)
{
    FillLine(0, ' ');
    FillLine(1, ' ');
    FillLine(2, ' ');
    FillLine(3, ' ');

    if (g_game.game_over == GAME_WIN) PutText(0, 0, "YOU WIN");
    else PutText(0, 0, "YOU LOSE");
    PutText(1, 0, "Time:");
    PutNum4(1, 5, Game_GetSurviveSecond() % 10000);
    PutText(2, 0, "Score:");
    PutNum4(2, 6, g_game.score % 10000);
    PutText(3, 0, "Best:");
    PutNum4(3, 5, Game_GetBestScore() % 10000);
    FlushDiff();
}

void UI_HandleKey(unsigned char key)
{
    if (g_ui_state == UI_START)
    {
        if (key == KEY_SHOOTER)
        {
            g_ui_state = UI_MODE;
            Beep_Bip(1, 40);
        }
        return;
    }

    if (g_ui_state == UI_MODE)
    {
        if (key == KEY_UP && g_mode_select > 0) g_mode_select--;
        if (key == KEY_DOWN && g_mode_select < 2) g_mode_select++;
        if (key == KEY_SHOOTER)
        {
            Game_Init(g_mode_select);
            g_ui_state = UI_PLAY;
            Beep_Bip(1, 50);
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
            Beep_Bip(1, 80);
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
