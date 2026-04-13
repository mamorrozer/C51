/*------------------------------------------------------------
 * 文件：game.h
 * 作用：游戏核心数据结构与逻辑接口定义（植物/敌人/子弹/全局状态）。
 * 框架：
 *   - 数据层：GameState + 三类对象数组
 *   - 逻辑层：初始化、输入处理、100ms推进、轨道字符构建
 *   - 存档层：最高分读写
 *-----------------------------------------------------------*/
#ifndef __GAME_H__
#define __GAME_H__

#define LANE_COUNT           3   /* 轨道数 */
#define MAP_COLS             12  /* 每条轨道可显示列数 */
#define MAX_PLANTS           8   /* 植物对象池容量 */
#define MAX_ENEMIES          8   /* 敌人对象池容量 */
#define MAX_BULLETS          12  /* 子弹对象池容量 */

#define PLANT_NONE           0
#define PLANT_WALL           1   /* 坚果墙 */
#define PLANT_SHOOTER        2   /* 射手 */

#define ENEMY_NONE           0
#define ENEMY_NORMAL         1   /* 普通僵尸 */
#define ENEMY_FAST           2   /* 快速僵尸 */

#define GAME_RUNNING         0   /* 进行中 */
#define GAME_WIN             1   /* 胜利 */
#define GAME_LOSE            2   /* 失败 */

typedef struct
{
    unsigned char active;  /* 是否激活 */
    unsigned char type;    /* 植物类型 */
    unsigned char lane;    /* 所在轨道 */
    unsigned char col;     /* 所在列 */
    unsigned char hp;      /* 血量 */
    unsigned char cool;    /* 射手冷却计数 */
} Plant;

typedef struct
{
    unsigned char active;    /* 是否激活 */
    unsigned char type;      /* 敌人类型 */
    unsigned char lane;      /* 所在轨道 */
    unsigned char x;         /* 横坐标 */
    unsigned char hp;        /* 血量 */
    unsigned char move_cnt;  /* 移动节拍计数 */
    unsigned char atk_cnt;   /* 攻击节拍计数 */
} Enemy;

typedef struct
{
    unsigned char active;  /* 是否激活 */
    unsigned char lane;    /* 所在轨道 */
    unsigned char x;       /* 横坐标 */
    unsigned char atk;     /* 攻击力 */
} Bullet;

typedef struct
{
    unsigned char cursor_lane;      /* 光标轨道 */
    unsigned char cursor_col;       /* 光标列 */
    unsigned char life;             /* 基地生命 */
    unsigned int resource;          /* 建造资源 */
    unsigned int score;             /* 当前分数 */
    unsigned int survive_tick;      /* 已生存tick（100ms单位） */
    unsigned int target_tick;       /* 目标生存tick */
    unsigned char spawn_interval;   /* 刷怪间隔（tick） */
    unsigned char enemy_move_step;  /* 普通敌移动步进阈值 */
    unsigned char game_over;        /* 胜负状态 */
} GameState;

extern GameState g_game;
#ifndef GAME_OBJECT_MEM
#define GAME_OBJECT_MEM xdata
#endif
extern Plant GAME_OBJECT_MEM g_plants[MAX_PLANTS];
extern Enemy GAME_OBJECT_MEM g_enemies[MAX_ENEMIES];
extern Bullet GAME_OBJECT_MEM g_bullets[MAX_BULLETS];

void Game_Init(unsigned char mode);
void Game_Update100ms(void);
void Game_HandleKey(unsigned char key);
/* 将指定轨道对象渲染为 12 字符（供 UI 显示）。 */
void Game_BuildLaneChars(unsigned char lane, char *out12);
/* 从 EEPROM 加载历史最高分。 */
void Game_LoadBestScore(void);
/* 若当前分数更高则写回 EEPROM。 */
void Game_SaveBestScoreIfNeed(void);
/* 读取缓存中的最高分。 */
unsigned int Game_GetBestScore(void);
/* 返回生存秒数（survive_tick / 10）。 */
unsigned int Game_GetSurviveSecond(void);

#endif
