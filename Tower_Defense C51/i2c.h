/*------------------------------------------------------------
 * 文件：i2c.h
 * 作用：软件 I2C 基础原语，供 OLED / AT24C02 / PCF8591 共用。
 * 框架：提供起始、停止、读写字节、读写应答位等低层接口。
 *-----------------------------------------------------------*/
#ifndef __I2C_H__
#define __I2C_H__

/*==================== I2C1：OLED/AT24C02（P0.0/P0.1） ====================*/
/* 起始信号：SCL高电平期间 SDA 高->低。 */
void I2C_Start(void);
/* 停止信号：SCL高电平期间 SDA 低->高。 */
void I2C_Stop(void);
/* 发送一个字节（MSB first）。 */
void I2C_Write(unsigned char dat);
/* 读取一个字节（MSB first）。 */
unsigned char I2C_Read(void);
/* 读取从机应答位：0=ACK，1=NACK。 */
unsigned char I2C_ReadAck(void);
/* 主机发送应答位：ack=0 发送ACK；ack=1 发送NACK。 */
void I2C_SendAck(unsigned char ack);

/*==================== I2C2：PCF8591（P1.0/P1.1） ====================*/
void I2C2_Start(void);
void I2C2_Stop(void);
void I2C2_Write(unsigned char dat);
unsigned char I2C2_Read(void);
unsigned char I2C2_ReadAck(void);
void I2C2_SendAck(unsigned char ack);

#endif
