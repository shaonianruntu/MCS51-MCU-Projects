#include<reg52.h>
#define uchar unsigned char 
#define uint unsigned int

sfr WDT_CONTR=0xe1;		//定义单片机新加入的看门狗寄存器,
sbit led1=P1^0;			

void delayms(uchar);	//ms级延时函数

void main()
{
	WDT_CONTR=0x35;	   	//D7  D6    D5     D4       D3    D2  D1   D0
				 		//--  -- EN_WDT CLR_WDT IDLE_WDT PS2  PS1  PS0
						//0	  0		1	   1	    0	  1	   0	1
	led1=0;				//点亮led
	delayms(500);
	led1=1;				//熄灭led
	while(1)
	{
		delayms(1000);	//1s延时
		WDT_CONTR=0x35;
	}
}

/*ms级延时函数*/
void delayms(uint xms)
{
	uint i,j;
	for(i=xms;i>0;i--)
		for(j=114;j>0;j--);
}	
