/*
同步移位脉冲由TXD(P3^0)引脚输出,由RXD(P3^1)引脚输入
设置单片机串行口工作模式0(非同步串口通信,用于与
外面的同步移位寄存器相连.),间隔模式发送0xaa,
用双示波器观察P3^0和P3^1口波形.
*/

#include<reg52.h>
#define uchar unsigned char 

void init();
void delayms(uchar);

void main()
{
	init();			//串行口初始化设置
	TI=0;			//发送中断标志位清零
	while(1)
	{	   
		SBUF=0xaa;
		delayms(1);	//1ms延时
	}
}

/*串行口中断服务*/
void ser0() interrupt 4
{
	TI=0;
}

void init()
{
	SCON=0;			//SCON: SM0 SM1 SM2 REN TB8 RB8 TI  RI 
                    //50h =  0   0   0   0   0   0  0   0 b
					//设置串口工作方式为0
	EA=1;			//开总中断
	ES=1;			//开串口中断
}

void delayms(uchar xms)
{
	uchar i,j;
	for(i=xms;i>0;i--)
		for(j=114;j>0;j--);
}