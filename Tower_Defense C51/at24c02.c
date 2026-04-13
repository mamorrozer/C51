/*------------------------------------------------------------
 * 文件：at24c02.c
 * 作用：AT24C02 EEPROM 的字节/字读写实现。
 * 框架：
 *   - 字节写：器件地址写 + 存储地址 + 数据 + 写周期等待
 *   - 字节读：先写入待读地址，再重复起始切换到读
 *   - 字读写：基于字节接口封装（低字节在低地址）
 *-----------------------------------------------------------*/
#include "at24c02.h"
#include "i2c.h"
#include "delay.h"

void AT24C02_WriteByte(unsigned char addr, unsigned char dat)
{
    /* EEPROM 随机地址写流程。 */
    I2C_Start();
    I2C_Write(0xA0);
    if (I2C_ReadAck()) goto stop;
    I2C_Write(addr);
    if (I2C_ReadAck()) goto stop;
    I2C_Write(dat);
    if (I2C_ReadAck()) goto stop;
    I2C_Stop();
    /* AT24C02 内部写周期典型 5ms，期间不可立即读回。 */
    DelayMs(5);
    return;

stop:
    I2C_Stop();
}

unsigned char AT24C02_ReadByte(unsigned char addr)
{
    unsigned char dat;
    /* 先发送目标地址（写方向），定位内部地址指针。 */
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
