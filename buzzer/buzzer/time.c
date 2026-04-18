#include <REGX52.H>
void timer()
{
	TMOD &= 0xF0;		
	TMOD |= 0x01;		
	TL0 = 0x18;	
	TH0 = 0xFC;		
	TF0 = 0;
	TR0 = 1;	
	ET0=1;
	EA=1;
	PT0=1;
}

void tim1_Init()
{
	TMOD &= 0x0F;		
	TMOD |= 0x10;		
	TL1 = 0x67;	
	TH1 = 0xFC;		
	TF1 = 0;
	TR1 = 1;	
	ET1=1;
	EA=1;
	PT1=0;
}