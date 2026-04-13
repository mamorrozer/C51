/*------------------------------------------------------------
 * 文件：ui.h
 * 作用：UI 状态机接口，负责各页面绘制与键盘事件分发。
 * 框架：
 *   - 状态常量 UI_START/UI_MODE/UI_PLAY/UI_PAUSE/UI_RESULT
 *   - 绘制接口 UI_Draw*
 *   - 输入分发 UI_HandleKey
 *-----------------------------------------------------------*/
#ifndef __UI_H__
#define __UI_H__

#include "game.h"

#define UI_START      0  /* 开始页 */
#define UI_MODE       1  /* 模式选择页 */
#define UI_PLAY       2  /* 游戏进行页 */
#define UI_PAUSE      3  /* 暂停页 */
#define UI_RESULT     4  /* 结算页 */

extern unsigned char g_ui_state;
extern unsigned char g_mode_select;

/* 初始化 UI 状态机和双缓冲。 */
void UI_Init(void);
/* 绘制开始页。 */
void UI_DrawStart(void);
/* 绘制难度选择页。 */
void UI_DrawMode(void);
/* 绘制游戏主界面。 */
void UI_DrawGame(void);
/* 绘制暂停页。 */
void UI_DrawPause(void);
/* 绘制结算页。 */
void UI_DrawResult(void);
/* 根据当前状态处理输入键值。 */
void UI_HandleKey(unsigned char key);

#endif
