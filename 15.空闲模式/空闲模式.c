#include<reg52.h>
#define uchar unsigned char
#define uint unsigned int
sbit dula=P2^6;				//段选
sbit wela=P2^7;				//位选

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

void init();				//中断服务初始化
void display(uchar);		//数码管显示定时器计数
void delayms(uint);			//一般延时

uchar num,num1;

void main()
{
	init();					//初始化
	while(1)
	{
		if(20==num)			//1s
		{
			num=0;
			num1++;
			if(6==num1)		//第6秒
			{
				ET0=0;		//关闭定时器,等待外部中断发生
				PCON=0x01;	//进入空闲状态

							/*
							PCON=0x01,空闲模式,此时定时器还会工作
							PCON=0x02,休眠模式,此时定时器不会工作

							PCON=0x02等价与PC0N=0x01加上ET0=0;
							*/
			}
		}
		display(num1);		//数码管显示定时器计数
	}
}

/*中断服务初始化*/
void init()
{
	TMOD=0x01;				//设置定时器0工作方式1(0000 0001)
	TH0=(65535-45872)/256;
	TL0=(65536-45872)%256;	//50ms
	EA=1;					//开总中断
	ET0=1;					//开定时器0中断
	EX0=1;					//开外部中断0
	EX1=1;					//开外部中断1
	TR0=1;					//启动定时器0
}

/*定时器T0服务*/
void T0_time() interrupt 1
{
	TH0=(65535-45872)/256;
	TL0=(65536-45872)%256;	//50ms
	num++;	
}

/*外部中断0服务(P3^2接口)*/
void EX_int0() interrupt 0	
{
	PCON=0;					//退出空闲状态
	ET0=1;					//打开定时器0中断
}

/*外部中断1服务(P3^3接口)*/
void EX_int1() interrupt 2
{
	PCON=0;					//退出空闲状态
	ET0=1;					//打开定时器0中断
}

/*数码管显示定时器计数*/
void display(uchar a)
{
	uchar shi,ge;
	shi=a/10;
	ge=a%10;
	
	dula=1;
	P0=dutable[shi];	   //送十位段选数据
	dula=0;				   
	P0=0xff;			   //送位选数据前关闭所有显示,防止打开位选锁存时																				
	wela=1;				   //原来段选数据通过位选锁存器造成混乱
	P0=0xfe;			   //送位选数据
	wela=0;
	delayms(5);			   //延时
	
	dula=1;
	P0=dutable[ge];		  //送个位段选数据
	dula=0;
	P0=0xff;
	wela=1;
	P0=0xfd;
	wela=0;
	delayms(5);	
}

/*一般延时函数*/
void delayms(uint xms)
{
	uint i,j;
	for(i=xms;i>0;i--)
		for(j=114;j>0;j--);
}