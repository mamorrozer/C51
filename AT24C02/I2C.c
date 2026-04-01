#include <REGX52.H>
#include "I2C.h"

/**
 * @file I2C.c
 * @brief I2C总线通信驱动实现
 * @details 实现I2C总线的基本通信时序，包括起始条件、停止条件、数据读写和应答处理
 */

/**
 * @def I2C_SCL
 * @brief I2C时钟线引脚定义
 * @note 连接到P2.1引脚
 */
sbit I2C_SCL = P2^1;
sbit I2C_SDA = P2^0;

/**
 * @brief 产生I2C起始条件
 * @details 在SCL高电平期间，SDA从高到低跳变，表示通信开始
 * @return 无
 */
void I2C_Start()
{
    I2C_SDA = 1;    // 确保SDA为高电平
    I2C_SCL = 1;    // 确保SCL为高电平
    I2C_SDA = 0;    // SDA从高到低跳变，产生起始信号
    I2C_SCL = 0;    // 拉低SCL，准备数据传输
}

/**
 * @brief 产生I2C停止条件
 * @details 在SCL高电平期间，SDA从低到高跳变，表示通信结束
 * @return 无
 */
void I2C_Stop()
{
    I2C_SDA = 0;    // 确保SDA为低电平
    I2C_SCL = 1;    // 拉高SCL
    I2C_SDA = 1;    // SDA从低到高跳变，产生停止信号
}

/**
 * @brief 向I2C总线写入一个字节
 * @details 高位在前，逐位发送数据
 * @param byte 要发送的字节数据
 * @return 无
 */
void I2C_Write(unsigned char byte)
{
    unsigned char i;
    for (i = 0; i < 8; i++)
    {
        // 依次取出字节的每一位，写入SDA线
        // 使用移位和与运算确保结果为0或1
        I2C_SDA = (byte >> (7 - i)) & 0x01;
        I2C_SCL = 1;    // 拉高SCL，从机在此时采样数据
        I2C_SCL = 0;    // 拉低SCL，准备发送下一位
    }
}

/**
 * @brief 从I2C总线读取一个字节
 * @details 高位在前，逐位接收数据
 * @return 读取到的字节数据
 */
unsigned char I2C_Read(void)
{
    unsigned char byte = 0, i;
    I2C_SDA = 1;        // 释放SDA总线，由从机驱动
    for (i = 0; i < 8; i++)
    {
        I2C_SCL = 1;    // 拉高SCL，在此时读取SDA状态
        if (I2C_SDA)
            byte |= (0x80 >> i);   // 如果SDA为高，置对应位为1
        I2C_SCL = 0;    // 拉低SCL，准备读取下一位
    }
    return byte;
}

/**
 * @brief 主机发送应答信号
 * @details 用于向从机表示数据接收状态
 * @param abit 应答位，0表示ACK（应答），1表示NACK（非应答）
 * @return 无
 */
void I2C_TACK(unsigned char abit)
{
    I2C_SDA = abit;     // 设置应答位状态
    I2C_SCL = 1;        // 产生时钟脉冲，从机读取应答位
    I2C_SCL = 0;        // 拉低SCL，结束应答
}

/**
 * @brief 主机读取从机应答信号
 * @details 用于判断从机是否正确接收数据
 * @return 应答状态，0表示ACK（应答），1表示NACK（非应答）
 */
unsigned char I2C_RACK(void)
{
    unsigned char ack;
    I2C_SDA = 1;        // 释放SDA总线，准备读取应答
    I2C_SCL = 1;        // 产生时钟脉冲
    ack = I2C_SDA;      // 读取SDA线状态，获取应答信号
    I2C_SCL = 0;        // 拉低SCL，结束应答读取
    return ack;
}