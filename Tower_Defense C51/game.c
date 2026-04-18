/*------------------------------------------------------------
 * 文件：game.c
 * 作用：塔防核心逻辑（对象管理、碰撞结算、胜负判定、存档分数）。
 * 框架：
 *   - 数据：g_game + 植物/敌人/子弹对象池
 *   - 输入：Game_HandleKey() 处理移动/建造/铲除
 *   - 推进：Game_Update100ms() 以 100ms 为固定帧更新
 *   - 输出：Game_BuildLaneChars() 生成 UI 轨道字符
 *   - 存档：Game_LoadBestScore() / Game_SaveBestScore() *
 *   - 随机：NextRand() 生成 [0,65535] 的伪随机数 *
 *   - 对象池：ClearAllObjects() 清空所有激活的植物/敌人/子弹 *
 *   - 刷怪：PickSpawnLane() 根据当前模式选择轨道 *
 *   - 结算：Game_CheckWinLose() 检查游戏胜负 *
 * 还有一些函数说明在.h文件注释
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
#define SKILL_COST_SHOOTER     15
#define SKILL_COST_WALL        20
#define SHOOTER_DETECT_RANGE   6

GameState g_game;
Plant GAME_OBJECT_MEM g_plants[MAX_PLANTS];
Enemy GAME_OBJECT_MEM g_enemies[MAX_ENEMIES];
Bullet GAME_OBJECT_MEM g_bullets[MAX_BULLETS];

static unsigned int g_best_score[3] = {0, 0, 0};
static unsigned char g_current_mode = 0;
static unsigned int rand_seed = 0x35A1;
static unsigned char spawn_cnt = 0;
static unsigned char resource_cnt = 0;
static unsigned char g_special_mode = 0;
/* 【为什么不直接用 stdlib 的 rand()】
C51 的 rand() 会额外占用几十字节 RAM 和较大 ROM，对本项目 256B RAM 不可接受。
LFSR 仅需一个 16 位种子（2 字节 RAM），且周期长、分布均匀，适合刷怪随机轨道。
*/
static unsigned int NextRand(void);

static unsigned char PickSpawnLane(void)
{
    /* 常规模式：三条轨道等概率刷怪。 */
    return (unsigned char)(NextRand() % LANE_COUNT);
}

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
/* 【设计原因】51 无可靠 malloc，动态分配会导致内存碎片，甚至跑着跑着跑飞。
采用对象池，定长数组+active 标志，保证：
内存占用恒定，不会随游戏进行膨胀；
无外部碎片，避免碎片化导致的异常死机。
*/

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

static bit IsFoggedCol(unsigned char col)
{
    unsigned char fog_begin;
    if (!g_game.special_mode || g_game.fog_cols == 0) return 0;
    fog_begin = (unsigned char)(MAP_COLS - g_game.fog_cols);
    if (col >= fog_begin) return 1;
    return 0;
}

static Enemy* FindEnemyFront(unsigned char lane, unsigned char x)
{
    unsigned char i;
    unsigned char detect_max = (unsigned char)(x + SHOOTER_DETECT_RANGE);
    Enemy *best = 0;
    if (detect_max >= MAP_COLS) detect_max = (MAP_COLS - 1);
    /* 找到指定位置右侧最近的敌人，供射手判断是否开火。 */
    for (i = 0; i < MAX_ENEMIES; i++)
    {
        if (!g_enemies[i].active || g_enemies[i].lane != lane) continue;
        if (IsFoggedCol(g_enemies[i].x)) continue;
        if (g_enemies[i].x >= x && g_enemies[i].x <= detect_max)
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
    unsigned char lane = PickSpawnLane();
    unsigned char type;
    unsigned char hp;
    /* 常规模式：全轨道使用相同敌人分布。 */
    type = (unsigned char)((NextRand() % 10 < 3) ? ENEMY_FAST : ENEMY_NORMAL);

    hp = (type == ENEMY_NORMAL) ? 3 : 2;

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
            g_enemies[i].hp = hp;
            break;
        }
    }
}

static bit SpawnBullet(unsigned char lane, unsigned char x)
{
    unsigned char i;
    /* 子弹池满时会丢弃本次发射请求。 */
    if (x >= MAP_COLS) return 0;
    for (i = 0; i < MAX_BULLETS; i++)
    {
        if (!g_bullets[i].active)
        {
            g_bullets[i].active = 1;
            g_bullets[i].lane = lane;
            g_bullets[i].x = x;
            g_bullets[i].atk = 1;
            return 1;
        }
    }
    return 0;
}

static void ActivateShooterSkill(Plant *p)
{
    unsigned char burst;
    unsigned char base_x;
    if (p->col + 1 < MAP_COLS) base_x = (unsigned char)(p->col + 1);
    else base_x = p->col;

    /* 射手技能：瞬发多发子弹，形成短时爆发。 */
    for (burst = 0; burst < 3; burst++)
    {
        unsigned char bx = (unsigned char)(base_x + burst);
        if (bx >= MAP_COLS) bx = (MAP_COLS - 1);
        (void)SpawnBullet(p->lane, bx);
    }
    Beep_Bip(2, 25);
}

static void ActivateWallSkill(Plant *p)
{
    unsigned char i;
    /* 击退范围是“自己这一行 + 相邻两行”，横向是植物前后近距离。 */
    for (i = 0; i < MAX_ENEMIES; i++)
    {
        unsigned char lane_diff;
        if (!g_enemies[i].active) continue;
        lane_diff = (g_enemies[i].lane > p->lane) ? (unsigned char)(g_enemies[i].lane - p->lane)
                                                  : (unsigned char)(p->lane - g_enemies[i].lane);
        if (lane_diff > 1) continue;
        if (g_enemies[i].x + 1 < p->col || g_enemies[i].x > p->col + 2) continue;

        if (g_enemies[i].x + 2 < MAP_COLS) g_enemies[i].x += 2;
        else g_enemies[i].x = MAP_COLS - 1;
        g_enemies[i].move_cnt = 0;
    }
    Beep_Bip(1, 25);
}

static void ActivatePlantSkill(Plant *p)
{
    unsigned int cost;
    /* 技能采用统一入口：先按植物类型判定耗费，再派发具体效果。 */
    if (p->type == PLANT_SHOOTER) cost = SKILL_COST_SHOOTER;
    else if (p->type == PLANT_WALL) cost = SKILL_COST_WALL;
    else return;

    if (g_game.resource < cost) return;
    g_game.resource -= cost;

    if (p->type == PLANT_SHOOTER) ActivateShooterSkill(p);
    else if (p->type == PLANT_WALL) ActivateWallSkill(p);
}

static void BuildPlantByType(unsigned char type)
{
    unsigned char i;
    unsigned int cost;

    if (type == PLANT_SHOOTER) cost = 40;
    else if (type == PLANT_WALL) cost = 30;
    else return;

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
            if (type == PLANT_SHOOTER) g_plants[i].hp = 4;
            else if (type == PLANT_WALL) g_plants[i].hp = 9;
            else g_plants[i].hp = 9;
            g_game.resource -= cost;
            g_game.cursor_hide_tick = CURSOR_HIDE_TICKS;
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
    }
}

static void PlantsAttack(void)
{
    unsigned char i;
    unsigned char shoot_x;
    Enemy *target;
    /* 每 100ms 扫描一次植物：射手冷却结束且射程内有目标就开火。 */
    for (i = 0; i < MAX_PLANTS; i++)
    {
        if (!g_plants[i].active) continue;
        if (g_plants[i].type == PLANT_SHOOTER)
        {
            if (g_plants[i].cool > 0)
            {
                g_plants[i].cool--;
                continue;
            }
            target = FindEnemyFront(g_plants[i].lane, g_plants[i].col);
            if (target != 0)
            {
                if (target->x <= (unsigned char)(g_plants[i].col + 1))
                {
                    if (target->hp > 1) target->hp--;
                    else
                    {
                        target->active = 0;
                        g_game.score += 10;
                        g_game.resource += 5;
                    }
                }
                else
                {
                    shoot_x = (unsigned char)(g_plants[i].col + 1);
                    if (shoot_x >= MAP_COLS) shoot_x = g_plants[i].col;
                    (void)SpawnBullet(g_plants[i].lane, shoot_x);
                }
                g_plants[i].cool = 1;
            }
        }
    }
}

static bit BulletTryHitAt(unsigned char bullet_idx, unsigned char x)
{
    unsigned char j;
    /* 单发子弹最多命中一个敌人：命中后立即 inactive。 */
    for (j = 0; j < MAX_ENEMIES; j++)
    {
        if (!g_enemies[j].active) continue;
        if (g_enemies[j].lane != g_bullets[bullet_idx].lane) continue;
        if (g_enemies[j].x == x)
        {
            if (g_enemies[j].hp > g_bullets[bullet_idx].atk)
            {
                g_enemies[j].hp -= g_bullets[bullet_idx].atk;
            }
            else
            {
                g_enemies[j].active = 0;
                g_game.score += 10;
                g_game.resource += 5;
            }
            g_bullets[bullet_idx].active = 0;
            return 1;
        }
    }
    return 0;
}

static void MoveBulletsAndHit(void)
{
    unsigned char i;
    /* 先判当前格命中，再移动后再判，避免贴脸目标漏判。 */
    for (i = 0; i < MAX_BULLETS; i++)
    {
        if (!g_bullets[i].active) continue;
        if (BulletTryHitAt(i, g_bullets[i].x)) continue;
        if (g_bullets[i].x + 1 >= MAP_COLS)
        {
            g_bullets[i].active = 0;
            continue;
        }
        g_bullets[i].x++;
        if (BulletTryHitAt(i, g_bullets[i].x))
        {
            continue; /* 命中后立即销毁子弹，不会穿透后续目标。 */
        }
    }
}

static void EnemiesAct(void)
{
    unsigned char i;
    /* 敌人行为优先级（保证规则稳定）：
     * 1) 与植物同格时优先攻击植物，不移动
     * 2) 否则按速度节拍前进
     * 3) 到达最左端时扣基地生命并移除自身
     */
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
                if (p->hp > 1)
                {
                    p->hp--;
                    Beep_Bip(1, 18);
                }
                else
                {
                    p->active = 0;
                    Beep_Bip(2, 25);
                    continue;
                }
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
            Beep_Bip(4, 80);
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
    unsigned char i;
    /* 从 EEPROM 分别读取三档难度最高分。 */
    for (i = 0; i < 3; i++)
    {
        g_best_score[i] = AT24C02_ReadWord((unsigned char)(i * 2));
        if (g_best_score[i] > MAX_VALID_SCORE) g_best_score[i] = 0;
    }
}

void Game_SaveBestScoreIfNeed(void)
{
    /* 只有本局分数更高才写回 EEPROM，减少写损耗。 */
    if (g_game.score > g_best_score[g_current_mode])
    {
        g_best_score[g_current_mode] = g_game.score;
        AT24C02_WriteWord((unsigned char)(g_current_mode * 2), g_best_score[g_current_mode]);
    }
}

void Game_SetSpecialMode(unsigned char on)
{
    /* 特殊模式选择存放在静态变量；真正生效在下一局 Game_Init。 */
    g_special_mode = on ? 1 : 0;
}

unsigned int Game_GetBestScore(void)
{
    return Game_GetBestScoreByMode(g_current_mode);
}

unsigned int Game_GetBestScoreByMode(unsigned char mode)
{
    return g_best_score[mode];
}

/* 【迷雾策略：逻辑屏蔽，不画贴图】
迷雾通过在UI层把右侧若干列统一写为‘?’实现“视野剥夺”，
同时在射手索敌时跳过这些列（逻辑层过滤），
因此不占用额外显存，复杂度 O(1)。
*/
void Game_Init(unsigned char mode)
{
    /* 开局重置所有“局内状态”；持久数据（最高分）不在这里清。 */
    g_game.cursor_lane = 0;
    g_game.cursor_col = 1;
    g_game.life = 8;
    g_game.resource = 100;
    g_game.score = 0;
    g_game.survive_tick = 0;
    g_game.selected_tool = TOOL_SHOOTER;
    g_game.cursor_hide_tick = 0;
    g_game.special_mode = g_special_mode;
    g_game.fog_cols = 0;
    g_game.game_over = GAME_RUNNING;
    g_current_mode = mode;

    if (mode == 0)
    {
        /* EASY：刷怪慢、敌人移动慢。 */
        g_game.life = 10;
        g_game.spawn_interval = 16;
        g_game.enemy_move_step = 4;
        g_game.target_tick = 500;
    }
    else if (mode == 1)
    {
        /* NORMAL：中等节奏。 */
        g_game.life = 8;
        g_game.spawn_interval = 12;
        g_game.enemy_move_step = 3;
        g_game.target_tick = 600;
    }
    else
    {
        /* HARD：刷怪更快，综合压力更高。 */
        g_game.life = 6;
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
    /* 输入处理分三层：
     * 1) 工具快捷切换
     * 2) 光标移动/铲除
     * 3) 确认键：优先触发技能，其次建造
     */
    if (key == KEY_TOOL_SHOOTER)
    {
        g_game.selected_tool = TOOL_SHOOTER;
        return;
    }
    if (key == KEY_TOOL_WALL)
    {
        g_game.selected_tool = TOOL_WALL;
        return;
    }
    if (key == KEY_TOOL_REMOVE)
    {
        g_game.selected_tool = TOOL_REMOVE;
        return;
    }

    /* 光标移动：限制在有效地图范围。 */
    if (key == KEY_UP && g_game.cursor_lane > 0) g_game.cursor_lane--;
    if (key == KEY_DOWN && g_game.cursor_lane < (LANE_COUNT - 1)) g_game.cursor_lane++;
    if (key == KEY_LEFT && g_game.cursor_col > 0) g_game.cursor_col--;
    if (key == KEY_RIGHT && g_game.cursor_col < CURSOR_MAX_COL) g_game.cursor_col++;

    if (key == KEY_REMOVE)
    {
        RemovePlantAtCursor();
        return;
    }
    if (key != KEY_SHOOTER)
    {
        return;
    }

    /* 光标落在已有植物上时，触发该植物技能。 */
    if (g_game.special_mode && g_game.cursor_col > 0)
    {
        Plant *p_skill = FindPlant(g_game.cursor_lane, g_game.cursor_col);
        if (p_skill != 0)
        {
            ActivatePlantSkill(p_skill);
            return;
        }
    }

    /* 左侧工具列是“选工具区”，不是战场建造区。 */
    if (g_game.cursor_col == 0)
    {
        if (g_game.cursor_lane == 0) g_game.selected_tool = TOOL_SHOOTER;
        else if (g_game.cursor_lane == 1) g_game.selected_tool = TOOL_WALL;
        else g_game.selected_tool = TOOL_REMOVE;
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
    else RemovePlantAtCursor();
}

/*   固定顺序很关键：
     * 植物先开火，再结算弹道，最后敌人行动。
     * 顺序改变会直接影响“先手/后手”体验与难度。
*/
void Game_Update100ms(void)
{
    /* 这是游戏主时钟：每 100ms 进入一次，推进一帧战斗。 */
    if (g_game.game_over != GAME_RUNNING) return;

    /* 先判定是否达到生存目标，再执行本帧战斗更新。 */
    g_game.survive_tick++;
    if (g_game.cursor_hide_tick > 0) g_game.cursor_hide_tick--;
    if (g_game.special_mode && g_game.fog_cols < (MAP_COLS / 2))
    {
        if ((g_game.survive_tick % 50) == 0) g_game.fog_cols++;
    }
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
    /* 这是“逻辑层 -> 显示层”的转换：
     * 先铺底，再按优先级覆盖：植物 -> 子弹 -> 敌人 -> 光标。
     * 因此同一格出现冲突时，最终看到的是优先级更高的对象。
     */
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

