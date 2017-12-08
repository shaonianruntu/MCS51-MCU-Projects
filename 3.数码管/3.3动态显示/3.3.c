#include<reg52.h>
#define uchar unsigned char
#define uint unsigned int
sbit dula=P2^6;
sbit wela=P2^7;				    
uchar code weitable[]={		       //位选显示数组
0x7f,0xbf,0xdf,0xef,		    
0xf7,0xfb,0xfd,0xfe};
uchar code dutable[]={		       //段选显示数组
0x3f,0x06,0x5b,0x4f,
0x66,0x6d,0x7d,0x07,
0x7f,0x6f,0x77,0x7c,
0x39,0x5e,0x79,0x71};
void delayms(uint xms);
void main()
{
	uchar num;
	while(1)
	{
		for(num=1;num<=8;num++)
		{
			dula=1;
			P0=dutable[num];		//送段选
			dula=0;
			P0=0xff;				//段选后消影
			wela=1;
			P0=weitable[num-1];		//送位选
			wela=0;
			delayms(500);			//若每次显示单个数字则为500,若一次显示所有数字则为1
		}
	}
}

void delayms(uint xms)
{
	uint i,j;
	for(i=xms;i>0;i--)
		for(j=110;j>0;j--);
}
                                                      