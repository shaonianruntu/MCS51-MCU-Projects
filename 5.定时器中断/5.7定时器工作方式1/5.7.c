/*
定时器方式1位为16位计数器,TH0和TL0都是8位,可转载256个数
所以对256求模
*/

/*
实验板时钟频率11.0592MHz,机器周期12*(1/11059200)=1.09us
若t=50ms,N=50000/1.09=45872,
*/

#include<reg52.h>
#define uchar unsigned char 
sbit led=P1^1;

void init();			//中断服务初始化函数

uchar num;

void main()
{
	init();					   //中断服务初始化
	while(1)
	{
		if(20==num)			   //1s
		{
			num=0;
			led=~led;
		}
	}
}

/*中断服务初始化函数*/
void init()
{
	TMOD=0x01;					//设置定时器T0工作方式1
	TH0=(65536-45872)/256;
	TL0=(65536-45872)%256;		//50ms
	EA=1;						//开总中断
	ET0=1;						//开定时器0中断
	TR0=1;						//启动定时器0
}

/*定时器T0中断服务函数*/
void T0_time() interrupt 1
{
	TH0=(65535-45872)/256;
	TL0=(65536-45872)%256;
	num++;
}