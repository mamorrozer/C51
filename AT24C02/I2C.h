#ifndef __I2C_H__
#define __I2C_H__

// I2C总线基本操作函数声明
void I2C_Start(void);              // 产生起始信号
void I2C_Stop(void);               // 产生停止信号
void I2C_Write(unsigned char byte); // 写一个字节到总线
unsigned char I2C_Read(void);       // 从总线读取一个字节
void I2C_TACK(unsigned char abit);  // 主机发送应答（0=ACK, 1=NACK）
unsigned char I2C_RACK(void);       // 主机读取从机应答

#endif