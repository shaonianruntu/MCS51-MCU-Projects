#include<reg52.h>
#define uchar unsigned char 
#define uint unsigned int 
sbit dula=P2^6;
sbit wela=P2^7;
uchar code table[]={					 //段选数据显示数组
0x3f,0x06,0x5b,0x4f,
0x66,0x6d,0x7d,0x07,
0x7f,0x6f,0x77,0x7c,
0x39,0x5e,0x79,0x71};
void delayms(uint xms);
void main()
{
	uchar num;
	wela=1;
	P0=0xfe;
	wela=0;
	while(1)
	{
		for(num=0;num<16;num++)			//循环语句，循环显示0到F十六个数字
		{
			dula=1;
			P0=table[num];
			dula=0;
			delayms(500);				//延时，防抖
		}
	}
}

void delayms(uint xms)
{
	uint i,j;
	for(i=xms;i>0;i--)
		for(j=110;j>0;j--);
}
			