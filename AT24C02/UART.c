#include <REGX52.H>

void UART_Init()
{
	SCON=0x50;
	PCON=0x80;
	TMOD &= 0x0F;
	TMOD |= 0x20;
	TL1 = 0xF4;
	TH1 = 0xF4;
	ET1 = 0;
	TR1 = 1;
  ES=1;
  EA=1;
}

void UART_Transfer(unsigned char a)
{
	SBUF=a;
	while(TI==0);
	TI=0;
}