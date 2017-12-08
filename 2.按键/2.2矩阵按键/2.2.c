#include<reg52.h>
#define uchar unsigned char
#define uint unsigned int
sbit dula=P2^6;
sbit wela=P2^7;

uchar code dutable[]={
0x3f,0x06,0x5b,0x4f,
0x66,0x6d,0x7d,0x07,
0x7f,0x6f,0x77,0x7c,
0x39,0x5e,0x79,0x71};

void matrixkeyscan();			   //矩阵按键识别子程序
void delayms(uint);				   //延时子程序
void display(uchar);			   //数码管显示子程序

void main()
{
	P0=0;
	dula=1;
	dula=0;
	P0=0;
	wela=1;
	wela=0;
	
	while(1)
	{
		matrixkeyscan();
	}
}

void matrixkeyscan()
{
	uchar temp,key;
	P3=0xfe;
	temp=P3;
	temp=temp&0xf0;
	if(temp!=0xf0)
	{
		delayms(10);
		temp=P3;
		temp=temp&0xf0;
		if(temp!=0xf0)
		{
			temp=P3;
			switch(temp)
			{
				case 0xee:
					key=1;
					break;
				case 0xde:
					key=2;
					break;
				case 0xbe:
					key=3;
					break;
				case 0x7e:
					key=4;
					break;
			}
			while(temp!=0xf0)
			{
				temp=P3;
				temp=temp&0xf0;
			}
			display(key);
		}
	}
	
	P3=0xfd;
	temp=P3;
	temp=temp&0xf0;
	if(temp!=0xf0)
	{
		delayms(10);
		temp=P3;
		temp=temp&0xf0;
		if(temp!=0xf0)
		{
			temp=P3;
			switch(temp)
			{
				case 0xed:
					key=5;
					break;
				case 0xdd:
					key=6;
					break;
				case 0xbd:
					key=7;
					break;
				case 0x7d:
					key=8;
					break;
			}
			while(temp!=0xf0)
			{
				temp=P3;
				temp=temp&0xf0;
			}
			display(key);
		}
	}
   
	P3=0xfb;
	temp=P3;
	temp=temp&0xf0;
	if(temp!=0xf0)
	{
		delayms(10);
		temp=P3;
		temp=temp&0xf0;
		if(temp!=0xf0)
		{
			temp=P3;
			switch(temp)
			{
				case 0xeb:
					key=9;
					break;
				case 0xdb:
					key=10;
					break;
				case 0xbb:
					key=11;
					break;
				case 0x7b:
					key=12;
					break;
			}
			while(temp!=0xf0)
			{
				temp=P3;
				temp=temp&0xf0;
			}
			display(key);
		}
	}

	P3=0xf7;
	temp=P3;
	temp=temp&0xf0;
	if(temp!=0xf0)
	{
		delayms(10);
		temp=P3;
		temp=temp&0xf0;
		if(temp!=0xf0)
		{
			temp=P3;
			switch(temp)
			{
				case 0xe7:
					key=13;
					break;
				case 0xd7:
					key=14;
					break;
				case 0xb7:
					key=15;
					break;
				case 0x77:
					key=16;
					break;
			}
			while(temp!=0xf0)
			{
				temp=P3;
				temp=temp&0xf0;
			}
			display(key);
		}
	}	
}

void delayms(uint xms)
{
	uint i,j;
	for(i=xms;i>0;i--)
		for(j=110;j>0;j--);
}

void display(uchar num)
{

	P0=dutable[num-1];
	dula=1;
	dula=0;
}	 