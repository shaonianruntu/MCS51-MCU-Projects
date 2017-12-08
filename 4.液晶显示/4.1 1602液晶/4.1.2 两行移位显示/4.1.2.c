#include<reg52.h>
#define uchar unsigned char
#define uint unsigned int
uchar code first_hang[]={"MY NAME IS:"};
uchar code second_hang[]={"FANG NAN"};
sbit dula=P2^6;			//U1锁存器的锁存端，段选
sbit wela=P2^7;			//U2锁存器的锁存端，位选
sbit lcden=P3^4;		//LCDEN液晶使能端
sbit lcdrs=P3^5;		//RS液晶数据命令选择端
sbit lcdwr=P3^6;		//WR液晶读写选择端

void init();
void write_com(uchar com);
void delayms(uint xms);
void write_data(uchar date);


void main()
{
	uchar num;
	init();
	write_com(0x80+0x10);
	for(num=0;num<11;num++)
	{
		write_data(first_hang[num]);
		delayms(5);
	}
	write_com(0x80+0x50);
	for(num=0;num<8;num++)
	{
		write_data(second_hang[num]);
		delayms(5);
	}
	for(num=0;num<16;num++)
	{
		write_com(0x18);		  //00011000B,整屏左移，同时光标跟随移动
		delayms(1000);
	}
	while(1);
}

void init()
{
	dula=0;
	wela=0;
	lcdwr=0;
	lcden=0;
	write_com(0x38);	 //00111000B,设置为16*2显示，5*7点阵，8位数据接口
	write_com(0x0c);	 //00001100B,设置开显示，不显示光标
	write_com(0x06);	 //00000110B,写一个字符后地址指针加1
	write_com(0x01);	 //00000001B,显示清0，数据指针清0
}

void write_com(uchar com)
{
	lcdrs=0;
	P0=com;
	delayms(5);
	lcden=1;
	delayms(5);
	lcden=0;
}

void delayms(uint xms)
{
	uint i;
	uchar j;
	for(i=xms;i>0;i--)
		for(j=110;j>0;j--);
}

void write_data(uchar date)
{
	lcdrs=1;
	P0=date;
	delayms(5);
	lcden=1;
	delayms(5);
	lcden=0;
}