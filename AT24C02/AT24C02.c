/**
 * @file AT24C02.c
 * @brief AT24C02 EEPROM驱动实现
 * @details 实现AT24C02 EEPROM的读写操作，包括字节写入和读取功能
 */

#include <REGX52.H>
#include "I2C.h"
#include "Delay.h"

/**
 * @brief 向AT24C02指定地址写入一字节数据
 * @details 通过I2C总线向AT24C02的指定地址写入一个字节的数据
 * @param where 存储地址（0-255）
 * @param byte 要写入的数据字节
 * @return 无
 * @note 写入操作后会等待5ms以确保内部写周期完成
 */
void AT24C02_WriteByte(unsigned char where, unsigned char byte)
{
    // 1. 发送起始信号
    I2C_Start();

    // 2. 发送设备地址 + 写操作（0xA0 = 1010 0000）
    I2C_Write(0xA0);
    if (I2C_RACK()) goto stop;   // 检查从机应答，失败则停止

    // 3. 发送存储地址（低8位，此处仅使用单字节地址）
    I2C_Write(where);
    if (I2C_RACK()) goto stop;

    // 4. 发送数据
    I2C_Write(byte);
    if (I2C_RACK()) goto stop;

    // 5. 发送停止信号
    I2C_Stop();
    Delay(5);   // 等待内部写周期完成（AT24C02典型5ms）
    return;

stop:
    // 若任何一步应答失败，立即停止总线，避免总线死锁
    I2C_Stop();
}

/**
 * @brief 从AT24C02指定地址读取一字节数据
 * @details 通过I2C总线从AT24C02的指定地址读取一个字节的数据
 * @param where 存储地址（0-255）
 * @return 读取到的数据字节，出错时返回0
 * @note 使用虚拟写操作定位地址，然后进行读操作
 */
unsigned char AT24C02_ReadByte(unsigned char where)
{
    unsigned char byte;

    // 虚拟写操作：定位到指定地址
    I2C_Start();
    I2C_Write(0xA0);           // 设备地址 + 写
    if (I2C_RACK()) goto stop;
    I2C_Write(where);          // 存储地址
    if (I2C_RACK()) goto stop;

    // 重新起始 + 读操作
    I2C_Start();
    I2C_Write(0xA1);           // 设备地址 + 读
    if (I2C_RACK()) goto stop;

    // 读取数据
    byte = I2C_Read();
    I2C_TACK(1);               // 主机发送非应答，结束读操作
    I2C_Stop();
    return byte;

stop:
    I2C_Stop();
    return 0;                  // 出错时返回0
}