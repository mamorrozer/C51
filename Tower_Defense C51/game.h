#ifndef __GAME_H__
#define __GAME_H__

#define LANE_COUNT           3
#define MAP_COLS             12
#define MAX_PLANTS           8
#define MAX_ENEMIES          8
#define MAX_BULLETS          12

#define PLANT_NONE           0
#define PLANT_WALL           1
#define PLANT_SHOOTER        2

#define ENEMY_NONE           0
#define ENEMY_NORMAL         1
#define ENEMY_FAST           2

#define GAME_RUNNING         0
#define GAME_WIN             1
#define GAME_LOSE            2

typedef struct
{
    unsigned char active;
    unsigned char type;
    unsigned char lane;
    unsigned char col;
    unsigned char hp;
    unsigned char cool;
} Plant;

typedef struct
{
    unsigned char active;
    unsigned char type;
    unsigned char lane;
    unsigned char x;
    unsigned char hp;
    unsigned char move_cnt;
    unsigned char atk_cnt;
} Enemy;

typedef struct
{
    unsigned char active;
    unsigned char lane;
    unsigned char x;
    unsigned char atk;
} Bullet;

typedef struct
{
    unsigned char cursor_lane;
    unsigned char cursor_col;
    unsigned char life;
    unsigned int resource;
    unsigned int score;
    unsigned int survive_tick;
    unsigned int target_tick;
    unsigned char spawn_interval;
    unsigned char enemy_move_step;
    unsigned char game_over;
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
void Game_BuildLaneChars(unsigned char lane, char *out12);
void Game_LoadBestScore(void);
void Game_SaveBestScoreIfNeed(void);
unsigned int Game_GetBestScore(void);
unsigned int Game_GetSurviveSecond(void);

#endif
