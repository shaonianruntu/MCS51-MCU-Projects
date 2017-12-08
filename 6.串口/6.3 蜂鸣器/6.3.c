#include<reg52.h>
#define uint unsigned int
#define uchar unsigned char 
sbit beep=P2^3;
uchar a,flag;

void init();

void main()
{
	init();
	while(1)
	{
		if(1==flag)
		{
			ES=0;			  //关闭串口中断
			if(a=='Y')		  //当输入数据为Y时开启蜂鸣器
				beep=0;
			if(a=='N')		  //当SBUF为N时关闭蜂鸣器
				beep=1;
			ES=1;			  //开启串口中断
			flag=0;			  //标志清零
		}
	}
}

void init()
{
	TMOD=0x20;			//设定T1定时器的工作方式为2
	TH1=0xfd;			//T1定时器装初值
	TL1=0xfd;
	TR1=1;				//启动T1定时器

	REN=1;			    //允许串口接收
	SM0=0;				//设定串口工作方式1
	SM1=1;
		
	EA=1;				//开总中断
	ES=1;				//开串口中断
}

void ser() interrupt 4
{
	RI=0;				//手动清零
	a=SBUF;				//接收数据
	flag=1;				//标志
}