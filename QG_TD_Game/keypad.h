#ifndef __KEYPAD_H__
#define __KEYPAD_H__

#define KEY_NONE      0
#define KEY_UP        2
#define KEY_LEFT      4
#define KEY_RIGHT     6
#define KEY_DOWN      8
#define KEY_SHOOTER   5
#define KEY_WALL      9
#define KEY_REMOVE    10
#define KEY_PAUSE     13
#define KEY_BACK      16

unsigned char Keypad_GetKey(void);

#endif
