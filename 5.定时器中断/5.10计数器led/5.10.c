/*
定时器0工作模式2，8位计数模式，用杜邦线一端接P1.1
另一端P3.4引脚，记录P1.1脉冲次数，每10次脉冲触发一次中断，
使计数器溢出产生中断点亮LED小灯*/
#include<reg52.h>
#define uint unsigned int
sbit led=P1^0;
sbit IN=P1^1;

void init();			  //中断服务初始化函数
void delayms(uint);		  //一般延时函数

void main()
{
	init();				   //中断服务初始化
	while(1)
	{
		IN=1;			  //置高脉冲输出IO口
		delayms(500);	  //0.5s延时
		IN=0;			  //置低脉冲输出IO口
		delayms(500);	  //0.5s延时
	}
}

/*中断服务初始化函数*/
void init()
{
	TMOD=0x06;			//设置计数器0为工作方式2,8位计数模式(0000 0110)
	TH0=256-10;			//TH储存8位重装初值
	TL0=256-10;			//TL储存8位初值
	EA=1;				//开总中断
	ET0=1;				//开定时器0中断
	TR0=1;				//启动定时器0;
}

/*一般延时*/
void delayms(uint xms)
{
	uint i,j;
	for(i=xms;i>0;i--)
		for(j=114;j>0;j--);
}

/*定时器0中断服务程序*/
void T0_time() interrupt 1
{
	led=~led;			//10次计数后中断,将led状态取反
}
