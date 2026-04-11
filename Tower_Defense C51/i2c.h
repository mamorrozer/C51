#ifndef __I2C_H__
#define __I2C_H__

void I2C_Start(void);
void I2C_Stop(void);
void I2C_Write(unsigned char dat);
unsigned char I2C_Read(void);
unsigned char I2C_ReadAck(void);
void I2C_SendAck(unsigned char ack);

#endif
