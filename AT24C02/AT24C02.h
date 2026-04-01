#ifndef __AT24C02_H__
#define __AT24C02_H__

// AT24C02 EEPROM操作函数声明
void AT24C02_WriteByte(unsigned char where, unsigned char byte); // 向指定地址写入一字节
unsigned char AT24C02_ReadByte(unsigned char where);              // 从指定地址读取一字节

#endif