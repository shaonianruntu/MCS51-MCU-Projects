#include<reg52.h>
#define uchar unsigned char
sbit led1=P1^0;
uchar num;

int main()
{
	TMOD=0x01;
	TH0=(65536-45872)/256;  	  //计时周期为50ms
	TL0=(65536-45872)%256;
	EA=1;						  //开总中断
	ET0=1;						  //开定时器0中断
	TR0=1;						  //启动定时器0
	while(1)
	{
		if(20==num)
		{
			num=0;
			led1=~led1;			   //发光管状态取反
		}
	}
}

void T0_time() interrupt 1
{
	TH0=(65536-45872)/256;
	TL0=(65536-45872)%256;
	num++;
}