/*------------------------------------------------------------
 * 文件：at24c02.h
 * 作用：AT24C02 EEPROM 读写接口，用于最高分持久化。
 * 框架：提供字节读写与16位字读写（低字节在前）接口。
 *-----------------------------------------------------------*/
#ifndef __AT24C02_H__
#define __AT24C02_H__

/* 写 1 字节到指定 EEPROM 地址。 */
void AT24C02_WriteByte(unsigned char addr, unsigned char dat);
/* 从指定 EEPROM 地址读 1 字节。 */
unsigned char AT24C02_ReadByte(unsigned char addr);
/* 以“小端顺序”写 16 位整数（低字节在低地址）。 */
void AT24C02_WriteWord(unsigned char addr, unsigned int dat);
/* 以“小端顺序”读 16 位整数。 */
unsigned int AT24C02_ReadWord(unsigned char addr);

#endif
