#include<reg52.h>
#define uchar unsigned char
#define uint unsigned int

sbit led1=P1^0;
sbit dula=P2^6;
sbit wela=P2^7;

uchar code dutable[]={
0x3f,0x06,0x5b,0x4f,
0x66,0x6d,0x7d,0x07,
0x7f,0x6f,0x77,0x7c,
0x39,0x5e,0x79,0x71};

uchar num0,num1,num2;
void display(uchar);
void delayms(uint);

void main()
{
	TMOD=0x11;					    //设置定时器0和定时器1的工作方式为1 (0001 0001)
	TH0=(65536-45872)/256;			//定时器周期为50ms
	TL0=(65536-45872)%256;
	EA=1;							//开总中断
	ET0=1;							//开定时器0中断
	ET1=1;							//开定时器1中断
	TR0=1;							//启动定时器0
	TR1=1;							//启动定时器1
	while(1)
	{
		if(20==num1)				//计满20次，为1s
		{
			num1=0;
			num2++;
		}
		if(60==num2)				//满60s，变为0
			num2=0;
		display(num2);
	}
}

void display(uchar num)
{
	uchar shi,ge;				    //分离各位数字
	shi=num/10;
	ge=num%10;
	
	dula=1;
	P0=dutable[shi];
	dula=0;
	P0=0xff;						   //消影
	wela=1;
	P0=0xbf;
	wela=0;
	delayms(5);

	dula=1;
	P0=dutable[ge];
	dula=0;
	P0=0xff;						   //消影
	wela=1;
	P0=0x7f;
	wela=0;
	delayms(5);
}

void delayms(uint xms)					//延时子程序
{
	uint i,j;
	for(i=xms;i>0;i--)
		for(j=110;j>0;j--);
}

void T0_time() interrupt 1
{
	TH0=(65536-45872)/256;				 //重装初值，50ms
	TL0=(65536-45872)%256;
	num0++;
	if(4==num0)					//计满4次，即200ms，led闪烁一次
	{
		num0=0;					  //T0与T1的中断可能存在冲突，所以T0的累加计数不能写在主函数
		led1=~led1;
	}
}

void T1_time() interrupt 3
{
	TH0=(65536-45872)/256;			    //重装初值，50ms
	TL0=(65536-45872)%256;
	num1++;
}