#include<reg52.h>
#define uchar unsigned char 
#define uint unsigned int 

uchar a,flag;

void init();

void main()
{
	init();
	while(1)
	{
		if(1==flag)
		{
			ES=0;					//关闭串口中断
			P1=a;					//点亮小灯
			ES=1;				    //开启串口中断
			flag=0;					//清零标志位
		}	
	}
}

void init()
{
	TMOD=0x20;						//设置定时器1的工作方式为2
	TH1=0xfd;						//T1定时器赋初值
	TL1=0xfd;						
	TR1=1;							//开启T1

	REN=1;							//允许串口接收
	SM0=0;							//设定串口工作方式1
	SM1=1;

	EA=1;							//开总中断
	ES=1;							//开串口中断
}

void ser() interrupt 4
{
	RI=0;						   //手动清零
	a=SBUF;						   //接收数据	
	flag=1;						   //标志是否已收到数据
}