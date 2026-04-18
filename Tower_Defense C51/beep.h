/*------------------------------------------------------------
 * 文件：beep.h
 * 作用：蜂鸣器驱动接口，支持初始化、触发蜂鸣、1ms节拍推进。
 * 框架：
 *   - Beep_Init()   : 硬件初始电平配置
 *   - Beep_Bip()    : 发起一次/多次蜂鸣请求
 *   - Beep_Tick()   : 在定时器中按毫秒推进蜂鸣状态机
 *-----------------------------------------------------------*/
#ifndef __BEEP_H__
#define __BEEP_H__

/* 配置蜂鸣器 IO 初始状态。 */
void Beep_Init(void);
/* 请求蜂鸣：times 为次数，ms 为首段持续时长。 */
void Beep_Bip(unsigned char times, unsigned int ms);
/* 1ms 调用一次，推进蜂鸣状态机。 */
void Beep_Tick(void);

#endif
