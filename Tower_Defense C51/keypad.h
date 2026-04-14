/*------------------------------------------------------------
 * 文件：keypad.h
 * 作用：输入抽象层，把矩阵键盘按键转换成游戏统一键值。
 * 框架：外部只关心 Keypad_GetKey() 返回的 KEY_* 语义码。
 *-----------------------------------------------------------*/
#ifndef __KEYPAD_H__
#define __KEYPAD_H__

#define KEY_NONE      0   /* 无输入 */
#define KEY_UP        2   /* 光标上移 */
#define KEY_LEFT      4   /* 光标左移 */
#define KEY_RIGHT     6   /* 光标右移 */
#define KEY_DOWN      8   /* 光标下移 */
#define KEY_SHOOTER   5   /* 确认/执行当前工具 */
#define KEY_WALL      9   /* 直接放置坚果墙（保留语义） */
#define KEY_REMOVE    10  /* 直接铲除植物（保留语义） */
#define KEY_PAUSE     13  /* 暂停/继续 */
#define KEY_BACK      16  /* 返回/退出 */
#define KEY_TOOL_SHOOTER 17 /* 快捷切换到射手工具 */
#define KEY_TOOL_WALL    18 /* 快捷切换到坚果墙工具 */
#define KEY_TOOL_REMOVE  19 /* 快捷切换到铲除工具 */

unsigned char Keypad_GetKey(void);

#endif
