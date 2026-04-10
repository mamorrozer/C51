#ifndef __AT24C02_H__
#define __AT24C02_H__

void AT24C02_WriteByte(unsigned char addr, unsigned char dat);
unsigned char AT24C02_ReadByte(unsigned char addr);
void AT24C02_WriteWord(unsigned char addr, unsigned int dat);
unsigned int AT24C02_ReadWord(unsigned char addr);

#endif
