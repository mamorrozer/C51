/*------------------------------------------------------------
 * 文件：keypad.h
 * 作用：输入抽象层，把摇杆方向/按压转换成游戏统一键值。
 * 框架：外部只关心 Keypad_GetKey() 返回的 KEY_* 语义码。
 *-----------------------------------------------------------*/
#ifndef __KEYPAD_H__
#define __KEYPAD_H__

#define KEY_NONE      0   /* 无输入 */
#define KEY_UP        2   /* 光标上移 */
#define KEY_LEFT      4   /* 光标左移 */
#define KEY_RIGHT     6   /* 光标右移 */
#define KEY_DOWN      8   /* 光标下移 */
#define KEY_SHOOTER   5   /* 放置射手 */
#define KEY_WALL      9   /* 放置坚果墙 */
#define KEY_REMOVE    10  /* 铲除植物 */
#define KEY_PAUSE     13  /* 暂停/继续 */
#define KEY_BACK      16  /* 返回/退出 */

unsigned char Keypad_GetKey(void);

#endif
