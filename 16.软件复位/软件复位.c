#include<reg52.h>
#define uint unsigned int
#define uchar unsigned char
sfr ISP_CONTR=0xe7;			//定义ISP/IAP控制寄存器
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

void init();	 //初始化函数
void display(uchar);//数码管显示函数
void delayms(uchar);//ms级延时函数

uchar num,num1;	

void main()
{
	init();
	while(1)
	{
		if(num>=20)		//1s
		{
			num=0;
			num1++;
			if(num1==10)	  //10s
			{
				ISP_CONTR=0x20;	//SWRST=1;产生软件系统复位	
			}
		}
		display(num1);
	}
}

/*系统初始化函数*/
void init()
{
	TMOD=0x01;	//设置定时器0的工作方式1
	TH0=(65536-45872)/256;
	TL0=(65536-45872)%256;	//50ms
	EA=1;		//开总中断	
	ET0=1;		//开定时器0中断
	TR0=1;		//启动T0
}

/*数码管显示函数*/
void display(uchar number)
{
	uchar shi,ge;
	shi=number/10;
	ge=number%10;

	dula=1;
	P0=dutable[shi];
	dula=0;
	P0=0xff;
	wela=1;
	P0=0xfe;
	wela=0;
	delayms(5);

	dula=1;
	P0=dutable[ge];
	dula=0;
	P0=0xff;
	wela=1;
	P0=0xfd;
	wela=0;
	delayms(5);
}

/*ms级延时函数*/
void delayms(uchar xms)
{
	uchar i,j;
	for(i=xms;i>0;i--)
		for(j=114;j>0;j--);
}

void T0_time() interrupt 1
{
	TH0=(65536-45872)/256;
	TL0=(65536-45872)%256;	//50ms
	num++;
}
