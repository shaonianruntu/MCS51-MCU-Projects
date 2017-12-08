#include<reg52.h>
#define uint unsigned int
#define uchar unsigned char 
uchar flag,a;
uchar code table[]={"I GET A "};

void init();

void main()
{
	uchar i;
	init();
	while(1)
	{
		if(1==flag)
		{
			ES=0;							 //关闭串口中断
			for(i=0;i<8;i++)
			{
				SBUF=table[i];				 //发送数据
				while(!TI);					 //等待是否发送完毕
				TI=0;						 //手动清0
			}
			SBUF=a;
			while(!TI);						 
			TI=0;
			ES=1;							  
			flag=0;
		}	
	}
}

void init()
{
	TMOD=0X20;				 //设定定时器1的工作方式为2
	TH1=0xfd;				 //T1定时器装初值
	TL1=0xfd;
	TR1=1;					 //启动定时器T1

	REN=1;					 //允许串口接收
	SM0=0;
	SM1=1;					 //设置串口的工作方式为1

	EA=1;					 //开总中断
	ES=1;					 //开串口中断
}

void ser() interrupt 4
{
	RI=0;					  //手动清0
	a=SBUF;					  //接收数据
	flag=1;					  //标志已接收
}