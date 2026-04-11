#include "at24c02.h"
#include "i2c.h"
#include "delay.h"

void AT24C02_WriteByte(unsigned char addr, unsigned char dat)
{
    I2C_Start();
    I2C_Write(0xA0);
    if (I2C_ReadAck()) goto stop;
    I2C_Write(addr);
    if (I2C_ReadAck()) goto stop;
    I2C_Write(dat);
    if (I2C_ReadAck()) goto stop;
    I2C_Stop();
    DelayMs(5);
    return;

stop:
    I2C_Stop();
}

unsigned char AT24C02_ReadByte(unsigned char addr)
{
    unsigned char dat;
    I2C_Start();
    I2C_Write(0xA0);
    if (I2C_ReadAck()) goto stop;
    I2C_Write(addr);
    if (I2C_ReadAck()) goto stop;

    I2C_Start();
    I2C_Write(0xA1);
    if (I2C_ReadAck()) goto stop;
    dat = I2C_Read();
    I2C_SendAck(1);
    I2C_Stop();
    return dat;

stop:
    I2C_Stop();
    return 0;
}

void AT24C02_WriteWord(unsigned char addr, unsigned int dat)
{
    AT24C02_WriteByte(addr, (unsigned char)(dat & 0x00FF));
    AT24C02_WriteByte(addr + 1, (unsigned char)(dat >> 8));
}

unsigned int AT24C02_ReadWord(unsigned char addr)
{
    unsigned int val = 0;
    val = AT24C02_ReadByte(addr + 1);
    val <<= 8;
    val |= AT24C02_ReadByte(addr);
    return val;
}
