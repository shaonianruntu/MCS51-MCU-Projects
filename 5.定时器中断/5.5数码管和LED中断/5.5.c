#include<reg52.h>
#include<intrins.h>
#define uchar unsigned char 
sbit dula=P2^6;
sbit wela=P2^7;

uchar code dutable[]={
0x3f,0x06,0x5b,0x4f,
0x66,0x6d,0x7d,0x07,
0x7f,0x6f,0x77,0x7c,
0x39,0x5e,0x79,0x71};

void init();
void display(uchar);
void delayms(uchar);

uchar time,time0,time1;		//time为数码管显示数字，time0,time1用于T0,T1定时器计数
uchar temp1=0x7f;			//LED

void main()
{
	
	init();
	while(1)
	{
		display(time);
	}
}

/*中断服务特殊功能寄存器配置*/
void init()
{
	TMOD=0x11;			//设定定时器T1、T0的计数工作方式为1
	TH0=TH1=(65536-45872)/256;
	TL0=TL1=(65536-45872)%256;		//设定T0、T1的定时周期为50ms
	ET0=ET1=1;			//开T1、T0中断
	TR0=TR1=1;			//启动定时器T1、T0中断
	
	EX0=1;				//开启外部中断
	IT0=0;				//外部中断0为低电平触发

	EA=1;				//开总中断
}

/*数码管显示函数*/
void display(uchar num)
{
	uchar shi,ge;		   //分离十位数和个位数
	shi=num/10;
	ge=num%10;

	dula=1;
	P0=dutable[shi];
	dula=0;
	P0=0xff;			  //消影
	wela=1;
	P0=0xfe;
	wela=0;
	delayms(5);			  //消影

	dula=1;
	P0=dutable[ge];
	dula=0;
	P0=0xff;
	wela=1;
	P0=0xfd;
	wela=0;
	delayms(5);
}

/*延时函数*/
void delayms(uchar xms)
{
	uchar i,j;
	for(i=xms;i>0;i--)
		for(j=115;j>0;j--);
}

/*T0定时器中断服务程序*/			 
void T0_time() interrupt 1
{
	TH0=(65536-45872)/256;
	TL0=(65536-45872)%256;		   //50ms
	time0++;
	if(20==time0)				   //1s
	{
		time0=0;
		time++;
	}
	if(60==time)	
		time=0;
}

/*T1定时器中断服务配置*/
void T1_time() interrupt 3
{
	TH1=(65536-45872)/256;		   //50ms
	TL1=(65536-45872)%256;
	time1++;
	if(10==time1)				   //500ms
	{
		time1=0;
		P1=temp1;
		temp1=_cror_(temp1,1);
	}
}

/*外部中断0中断服务配置*/
void int0() interrupt 0
{
	TR0=0;
} 