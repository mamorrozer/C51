#include <REGX52.H>
#include "delay.h"
unsigned int anjian()
{
	unsigned int ji1=0;
	P1=0xFF;
	P1_3=0;
	if(P1_7==0){Delay(20);while(P1_7==0);Delay(20);ji1=1;}
	if(P1_6==0){Delay(20);while(P1_6==0);Delay(20);ji1=5;}
	if(P1_5==0){Delay(20);while(P1_5==0);Delay(20);ji1=9;}
	if(P1_4==0){Delay(20);while(P1_4==0);Delay(20);ji1=13;}
	
	P1=0xFF;
	P1_2=0;
	if(P1_7==0){Delay(20);while(P1_7==0);Delay(20);ji1=2;}
	if(P1_6==0){Delay(20);while(P1_6==0);Delay(20);ji1=6;}
	if(P1_5==0){Delay(20);while(P1_5==0);Delay(20);ji1=10;}
	if(P1_4==0){Delay(20);while(P1_4==0);Delay(20);ji1=14;}
	
	P1=0xFF;
	P1_1=0;
	if(P1_7==0){Delay(20);while(P1_7==0);Delay(20);ji1=3;}
	if(P1_6==0){Delay(20);while(P1_6==0);Delay(20);ji1=7;}
	if(P1_5==0){Delay(20);while(P1_5==0);Delay(20);ji1=11;}
	if(P1_4==0){Delay(20);while(P1_4==0);Delay(20);ji1=15;}
	
	P1=0xFF;
	P1_0=0;
	if(P1_7==0){Delay(20);while(P1_7==0);Delay(20);ji1=4;}
	if(P1_6==0){Delay(20);while(P1_6==0);Delay(20);ji1=8;}
	if(P1_5==0){Delay(20);while(P1_5==0);Delay(20);ji1=12;}
	if(P1_4==0){Delay(20);while(P1_4==0);Delay(20);ji1=16;}
	return ji1;
}