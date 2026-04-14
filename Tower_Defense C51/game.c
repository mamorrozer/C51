/*------------------------------------------------------------
 * 文件：game.c
 * 作用：塔防核心逻辑（对象管理、碰撞结算、胜负判定、存档分数）。
 * 框架：
 *   - 数据：g_game + 植物/敌人/子弹对象池
 *   - 输入：Game_HandleKey() 处理移动/建造/铲除
 *   - 推进：Game_Update100ms() 以 100ms 为固定帧更新
 *   - 输出：Game_BuildLaneChars() 生成 UI 轨道字符
 *-----------------------------------------------------------*/
#include "game.h"
#include "keypad.h"
#include "beep.h"
#include "at24c02.h"

#define CURSOR_MAX_COL  (MAP_COLS - 1)
#define MAX_VALID_SCORE 9999u
#define FAST_ENEMY_MOVE_STEP 3
#define RESOURCE_GAIN_INTERVAL 10
#define RESOURCE_GAIN_AMOUNT   3
#define CURSOR_HIDE_TICKS      5

GameState g_game;
Plant GAME_OBJECT_MEM g_plants[MAX_PLANTS];
Enemy GAME_OBJECT_MEM g_enemies[MAX_ENEMIES];
Bullet GAME_OBJECT_MEM g_bullets[MAX_BULLETS];

static unsigned int g_best_score = 0;
static unsigned int rand_seed = 0x35A1;
static unsigned char spawn_cnt = 0;
static unsigned char resource_cnt = 0;

static unsigned int NextRand(void)
{
    /* 16位 Galois LFSR，多项式掩码 0xB400，周期可达 65535。 */
    rand_seed = (rand_seed >> 1) ^ (-(rand_seed & 1u) & 0xB400u);
    return rand_seed;
}

static void ClearAllObjects(void)
{
    unsigned char i;
    /* 对象池清空：把 active 统一置0即可视为释放。 */
    for (i = 0; i < MAX_PLANTS; i++) g_plants[i].active = 0;
    for (i = 0; i < MAX_ENEMIES; i++) g_enemies[i].active = 0;
    for (i = 0; i < MAX_BULLETS; i++) g_bullets[i].active = 0;
}

static Plant* FindPlant(unsigned char lane, unsigned char col)
{
    unsigned char i;
    /* 按轨道+列定位植物，用于碰撞检测与建造判重。 */
    for (i = 0; i < MAX_PLANTS; i++)
    {
        if (g_plants[i].active && g_plants[i].lane == lane && g_plants[i].col == col)
        {
            return &g_plants[i];
        }
    }
    return 0;
}

static Enemy* FindEnemyFront(unsigned char lane, unsigned char x)
{
    unsigned char i;
    Enemy *best = 0;
    /* 找到指定位置右侧最近的敌人，供射手判断是否开火。 */
    for (i = 0; i < MAX_ENEMIES; i++)
    {
        if (!g_enemies[i].active || g_enemies[i].lane != lane) continue;
        if (g_enemies[i].x >= x)
        {
            if (best == 0 || g_enemies[i].x < best->x)
            {
                best = &g_enemies[i];
            }
        }
    }
    return best;
}

static void SpawnEnemy(void)
{
    unsigned char i;
    unsigned char lane = (unsigned char)(NextRand() % LANE_COUNT);
    unsigned char type = (unsigned char)((NextRand() & 0x01) ? ENEMY_NORMAL : ENEMY_FAST);
    /* 生成策略：随机轨道 + 随机类型，放到最右侧出生点。 */
    for (i = 0; i < MAX_ENEMIES; i++)
    {
        if (!g_enemies[i].active)
        {
            g_enemies[i].active = 1;
            g_enemies[i].lane = lane;
            g_enemies[i].type = type;
            g_enemies[i].x = MAP_COLS - 1;
            g_enemies[i].move_cnt = 0;
            g_enemies[i].atk_cnt = 0;
            g_enemies[i].hp = (type == ENEMY_NORMAL) ? 3 : 2;
            break;
        }
    }
}

static void SpawnBullet(unsigned char lane, unsigned char x)
{
    unsigned char i;
    /* 子弹池满时会丢弃本次发射请求。 */
    if (x >= MAP_COLS) return;
    for (i = 0; i < MAX_BULLETS; i++)
    {
        if (!g_bullets[i].active)
        {
            g_bullets[i].active = 1;
            g_bullets[i].lane = lane;
            g_bullets[i].x = x;
            g_bullets[i].atk = 1;
            return;
        }
    }
}

static void BuildPlantByType(unsigned char type)
{
    unsigned char i;
    unsigned int cost;

    if (type == PLANT_SHOOTER) cost = 40;
    else cost = 30;

    if (g_game.cursor_col == 0) return; /* 最左列用于工具选择，不允许建造。 */
    if (g_game.resource < cost) return;
    if (FindPlant(g_game.cursor_lane, g_game.cursor_col) != 0) return;

    for (i = 0; i < MAX_PLANTS; i++)
    {
        if (!g_plants[i].active)
        {
            g_plants[i].active = 1;
            g_plants[i].type = type;
            g_plants[i].lane = g_game.cursor_lane;
            g_plants[i].col = g_game.cursor_col;
            g_plants[i].cool = 0;
            g_plants[i].hp = (type == PLANT_SHOOTER) ? 4 : 9;
            g_game.resource -= cost;
            g_game.cursor_hide_tick = CURSOR_HIDE_TICKS;
            Beep_Bip(1, 50);
            return;
        }
    }
}

static void RemovePlantAtCursor(void)
{
    Plant *p;
    if (g_game.cursor_col == 0) return; /* 最左列用于工具选择，不执行铲除。 */
    p = FindPlant(g_game.cursor_lane, g_game.cursor_col);
    if (p != 0)
    {
        p->active = 0;
        Beep_Bip(1, 30);
    }
}

static void PlantsAttack(void)
{
    unsigned char i;
    unsigned char shoot_x;
    /* 每帧遍历射手：冷却完成且前方有敌人则发射。 */
    for (i = 0; i < MAX_PLANTS; i++)
    {
        if (!g_plants[i].active) continue;
        if (g_plants[i].type != PLANT_SHOOTER) continue;

        if (g_plants[i].cool > 0)
        {
            g_plants[i].cool--;
            continue;
        }
        if (FindEnemyFront(g_plants[i].lane, g_plants[i].col) != 0)
        {
            shoot_x = (unsigned char)(g_plants[i].col + 1);
            if (shoot_x >= MAP_COLS) shoot_x = g_plants[i].col;
            SpawnBullet(g_plants[i].lane, shoot_x);
            g_plants[i].cool = 1;
            Beep_Bip(1, 20);
        }
    }
}

static void MoveBulletsAndHit(void)
{
    unsigned char i, j;
    /* 先移动子弹，再进行同轨道同坐标碰撞判定。 */
    for (i = 0; i < MAX_BULLETS; i++)
    {
        if (!g_bullets[i].active) continue;
        if (g_bullets[i].x + 1 >= MAP_COLS)
        {
            g_bullets[i].active = 0;
            continue;
        }
        g_bullets[i].x++;

        for (j = 0; j < MAX_ENEMIES; j++)
        {
            if (!g_enemies[j].active) continue;
            if (g_enemies[j].lane != g_bullets[i].lane) continue;
            if (g_enemies[j].x == g_bullets[i].x)
            {
                if (g_enemies[j].hp > g_bullets[i].atk)
                {
                    g_enemies[j].hp -= g_bullets[i].atk;
                }
                else
                {
                    g_enemies[j].active = 0;
                    g_game.score += 10;
                    g_game.resource += 5;
                    Beep_Bip(1, 40);
                }
                g_bullets[i].active = 0;
                break;
            }
        }
    }
}

static void EnemiesAct(void)
{
    unsigned char i;
    /* 敌人行为优先级：同格啃植物 > 按速度前进 > 到达左端扣血。 */
    for (i = 0; i < MAX_ENEMIES; i++)
    {
        Plant *p;
        unsigned char move_need;
        if (!g_enemies[i].active) continue;

        p = FindPlant(g_enemies[i].lane, g_enemies[i].x);
        if (p != 0)
        {
            g_enemies[i].atk_cnt++;
            if (g_enemies[i].atk_cnt >= 3)
            {
                g_enemies[i].atk_cnt = 0;
                if (p->hp > 1) p->hp--;
                else p->active = 0;
                Beep_Bip(1, 20);
            }
            continue;
        }

        /* 快速敌固定为每 300ms 移动一次，避免过快导致不可玩。 */
        move_need = (g_enemies[i].type == ENEMY_FAST) ? FAST_ENEMY_MOVE_STEP : g_game.enemy_move_step;
        g_enemies[i].move_cnt++;
        if (g_enemies[i].move_cnt < move_need) continue;
        g_enemies[i].move_cnt = 0;

        if (g_enemies[i].x == 0)
        {
            g_enemies[i].active = 0;
            if (g_game.life > 0) g_game.life--;
            Beep_Bip(2, 60);
            if (g_game.life == 0)
            {
                g_game.game_over = GAME_LOSE;
            }
            continue;
        }

        g_enemies[i].x--;
    }
}

void Game_LoadBestScore(void)
{
    /* 从 EEPROM 读取历史最高分，并做上限兜底。 */
    g_best_score = AT24C02_ReadWord(0);
    if (g_best_score > MAX_VALID_SCORE) g_best_score = 0;
}

void Game_SaveBestScoreIfNeed(void)
{
    /* 只有本局分数更高才写回 EEPROM，减少写损耗。 */
    if (g_game.score > g_best_score)
    {
        g_best_score = g_game.score;
        AT24C02_WriteWord(0, g_best_score);
    }
}

unsigned int Game_GetBestScore(void)
{
    return g_best_score;
}

void Game_Init(unsigned char mode)
{
    /* 每次开局都重置玩家状态与对象池。 */
    g_game.cursor_lane = 0;
    g_game.cursor_col = 1;
    g_game.life = 5;
    g_game.resource = 100;
    g_game.score = 0;
    g_game.survive_tick = 0;
    g_game.selected_tool = TOOL_SHOOTER;
    g_game.cursor_hide_tick = 0;
    g_game.game_over = GAME_RUNNING;

    if (mode == 0)
    {
        /* 简单：刷怪慢、敌人移动慢、目标时长较短。 */
        g_game.spawn_interval = 16;
        g_game.enemy_move_step = 4;
        g_game.target_tick = 500;
    }
    else
    {
        /* 困难：刷怪更快，综合压力更高。 */
        g_game.spawn_interval = 9;
        g_game.enemy_move_step = 3;
        g_game.target_tick = 700;
    }

    spawn_cnt = 0;
    resource_cnt = 0;
    ClearAllObjects();
}

void Game_HandleKey(unsigned char key)
{
    if (key == KEY_TOOL_SHOOTER)
    {
        g_game.selected_tool = TOOL_SHOOTER;
        Beep_Bip(1, 30);
        return;
    }
    if (key == KEY_TOOL_WALL)
    {
        g_game.selected_tool = TOOL_WALL;
        Beep_Bip(1, 30);
        return;
    }
    if (key == KEY_TOOL_REMOVE)
    {
        g_game.selected_tool = TOOL_REMOVE;
        Beep_Bip(1, 30);
        return;
    }

    /* 光标移动：限制在有效地图范围。 */
    if (key == KEY_UP && g_game.cursor_lane > 0) g_game.cursor_lane--;
    if (key == KEY_DOWN && g_game.cursor_lane < (LANE_COUNT - 1)) g_game.cursor_lane++;
    if (key == KEY_LEFT && g_game.cursor_col > 0) g_game.cursor_col--;
    if (key == KEY_RIGHT && g_game.cursor_col < CURSOR_MAX_COL) g_game.cursor_col++;

    if (key == KEY_WALL)
    {
        BuildPlantByType(PLANT_WALL);
        return;
    }
    if (key == KEY_REMOVE)
    {
        RemovePlantAtCursor();
        return;
    }
    if (key != KEY_SHOOTER)
    {
        return;
    }

    /* 光标在最左列时，中键用于选择工具，不执行建造。 */
    if (g_game.cursor_col == 0)
    {
        if (g_game.cursor_lane == 0) g_game.selected_tool = TOOL_SHOOTER;
        else if (g_game.cursor_lane == 1) g_game.selected_tool = TOOL_WALL;
        else g_game.selected_tool = TOOL_REMOVE;
        Beep_Bip(1, 30);
        return;
    }

    if (g_game.selected_tool == TOOL_SHOOTER)
    {
        BuildPlantByType(PLANT_SHOOTER);
    }
    else if (g_game.selected_tool == TOOL_WALL)
    {
        BuildPlantByType(PLANT_WALL);
    }
    else
    {
        RemovePlantAtCursor();
    }
}

void Game_Update100ms(void)
{
    /* 仅在运行态推进，胜负已定时不再更新对象。 */
    if (g_game.game_over != GAME_RUNNING) return;

    /* 先判定是否达到生存目标，再执行本帧战斗更新。 */
    g_game.survive_tick++;
    if (g_game.cursor_hide_tick > 0) g_game.cursor_hide_tick--;
    if (g_game.survive_tick >= g_game.target_tick)
    {
        g_game.game_over = GAME_WIN;
        g_game.score += 100;
        return;
    }

    spawn_cnt++;
    if (spawn_cnt >= g_game.spawn_interval)
    {
        spawn_cnt = 0;
        SpawnEnemy();
    }

    resource_cnt++;
    if (resource_cnt >= RESOURCE_GAIN_INTERVAL)
    {
        resource_cnt = 0;
        g_game.resource += RESOURCE_GAIN_AMOUNT;
    }

    /* 固定更新顺序：植物攻击 -> 子弹移动命中 -> 敌人行动。 */
    PlantsAttack();
    MoveBulletsAndHit();
    EnemiesAct();
}

unsigned int Game_GetSurviveSecond(void)
{
    return g_game.survive_tick / 10;
}

void Game_BuildLaneChars(unsigned char lane, char *out12)
{
    unsigned char i;
    unsigned char tool = (lane == 0) ? TOOL_SHOOTER : ((lane == 1) ? TOOL_WALL : TOOL_REMOVE);
    /* 先填背景，再按植物/子弹/敌人/光标覆盖，后者优先级更高。 */
    for (i = 0; i < MAP_COLS; i++) out12[i] = '-';
    out12[0] = (tool == g_game.selected_tool) ? ((lane == 0) ? 'S' : ((lane == 1) ? 'W' : 'R'))
                                              : ((lane == 0) ? 's' : ((lane == 1) ? 'w' : 'r'));

    for (i = 0; i < MAX_PLANTS; i++)
    {
        if (!g_plants[i].active || g_plants[i].lane != lane) continue;
        if (g_plants[i].col > 0 && g_plants[i].col < MAP_COLS)
        {
            out12[g_plants[i].col] = (g_plants[i].type == PLANT_SHOOTER) ? 'S' : 'W';
        }
    }
    for (i = 0; i < MAX_BULLETS; i++)
    {
        if (!g_bullets[i].active || g_bullets[i].lane != lane) continue;
        if (g_bullets[i].x > 0 && g_bullets[i].x < MAP_COLS)
        {
            out12[g_bullets[i].x] = '*';
        }
    }
    for (i = 0; i < MAX_ENEMIES; i++)
    {
        if (!g_enemies[i].active || g_enemies[i].lane != lane) continue;
        if (g_enemies[i].x > 0 && g_enemies[i].x < MAP_COLS)
        {
            out12[g_enemies[i].x] = (g_enemies[i].type == ENEMY_FAST) ? 'Z' : 'z';
        }
    }

    if (g_game.cursor_hide_tick == 0 &&
        g_game.cursor_lane == lane &&
        g_game.cursor_col < MAP_COLS)
    {
        out12[g_game.cursor_col] = '+';
    }
}

