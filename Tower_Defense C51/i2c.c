#include <REGX52.H>
#include "i2c.h"

/* I2C 总线统一改到 P0 口：OLED / AT24C02 / PCF8591 共用同一条总线。 */
/* 注意：8051 的 P0 为开漏结构，硬件上必须给 SCL/SDA 外接上拉电阻。 */
sbit I2C_SCL = P0^0;
sbit I2C_SDA = P0^1;

void I2C_Start(void)
{
    /* 起始条件：SCL 为高时，SDA 从高拉低。 */
    I2C_SDA = 1;
    I2C_SCL = 1;
    I2C_SDA = 0;
    I2C_SCL = 0;
}

void I2C_Stop(void)
{
    /* 停止条件：SCL 为高时，SDA 从低拉高。 */
    I2C_SDA = 0;
    I2C_SCL = 1;
    I2C_SDA = 1;
}

void I2C_Write(unsigned char dat)
{
    unsigned char i;
    /* 逐位发送 8bit 数据，先发高位。 */
    for (i = 0; i < 8; i++)
    {
        I2C_SDA = (dat & 0x80) ? 1 : 0;
        dat <<= 1;
        I2C_SCL = 1;
        I2C_SCL = 0;
    }
}

unsigned char I2C_Read(void)
{
    unsigned char i;
    unsigned char dat = 0;
    /* 释放 SDA，由从设备驱动数据线。 */
    I2C_SDA = 1;
    for (i = 0; i < 8; i++)
    {
        dat <<= 1;
        I2C_SCL = 1;
        if (I2C_SDA)
        {
            dat |= 0x01;
        }
        I2C_SCL = 0;
    }
    return dat;
}

unsigned char I2C_ReadAck(void)
{
    unsigned char ack;
    /* 读应答位：0=ACK，1=NACK。 */
    I2C_SDA = 1;
    I2C_SCL = 1;
    ack = I2C_SDA;
    I2C_SCL = 0;
    return ack;
}

void I2C_SendAck(unsigned char ack)
{
    /* 主机发送应答位：ack=0 发 ACK，ack=1 发 NACK。 */
    I2C_SDA = ack;
    I2C_SCL = 1;
    I2C_SCL = 0;
}
