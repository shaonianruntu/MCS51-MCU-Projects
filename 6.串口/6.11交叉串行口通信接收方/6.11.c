#include<reg52.h>
#define uchar unsigned char
#define uint unsigned int
sbit dula=P2^6;
sbit wela=P2^7;

uchar code dutable[]={		//共阴极数码管段选码
//0   1    2	3 
0x3f,0x06,0x5b,0x4f,
//4   5	   6 	7
0x66,0x6d,0x7d,0x07,
//8   9	   A	b
0x7f,0x6f,0x77,0x7c,
//C   d	   E	F
0x39,0x5e,0x79,0x71,
//-   .  关显示
0x40,0x80,0x00};

void init();		//串口通信服务初始化
void display(uchar);		//数码管显示函数

void main()
{
	init();			//初始化
	P0=0xfe;		//位选中第一位数码管
	wela=1;
	wela=0;
	while(1);
}

/*串口中断服务*/
void ser0() interrupt 4
{
	uchar a;		//寄存串口接收到的数据
	RI=0;			//接收中断标志位清零
	a=SBUF;			//获取串口数据
	dispaly(a);		//数码管显示
}

/*串口通信服务初始化函数*/
void init()
{
	TMOD=0x20;		//设置定时器1的工作方式2
	TH1=253;		//比特率9600
	TL1=253;		
	TR1=1;			//启动定时器T1

	SM0=0;
	SM1=1;			//串口工作方式1
	REN=1;			//允许串口接收数据

	EA=1;			//开总中断
	ES=1;			//开串口中断
}

/*数码管显示函数*/
void display(uchar num)
{
	P0=dutable[num];
	dula=1;
	dula=0;
}