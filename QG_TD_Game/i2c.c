#include <REGX52.H>
#include "i2c.h"

sbit I2C_SCL = P2^1;
sbit I2C_SDA = P2^0;

void I2C_Start(void)
{
    I2C_SDA = 1;
    I2C_SCL = 1;
    I2C_SDA = 0;
    I2C_SCL = 0;
}

void I2C_Stop(void)
{
    I2C_SDA = 0;
    I2C_SCL = 1;
    I2C_SDA = 1;
}

void I2C_Write(unsigned char dat)
{
    unsigned char i;
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
    I2C_SDA = 1;
    I2C_SCL = 1;
    ack = I2C_SDA;
    I2C_SCL = 0;
    return ack;
}

void I2C_SendAck(unsigned char ack)
{
    I2C_SDA = ack;
    I2C_SCL = 1;
    I2C_SCL = 0;
}
