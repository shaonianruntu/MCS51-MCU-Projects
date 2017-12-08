/*
方式2和方式3都为11位异步通信口,数据位为9位,有一位附加
位TB8或RB8.
2和3唯一的区别在于传输速率不同
*/

/*
设置单片机串行口工作模式为2,间隔循环发送十六进制数
0xaa,然后用示波器观察单片机P3^1口波形.
通过修改TB8的值,为0或1,比较波形
*/

#include<reg52.h>
#define uchar unsigned char

void init();		 //串行口服务初始化设置
void delayms(uchar); //一般延时函数

void main()
{
	init();
	TI=0;
	{
		SBUF=0xaa;
		delayms(1);

	}
}

void ser0() interrupt 4
{
	TI=0;
}
/*串行口服务初始化设置*/
void init()
{
	SM0=1;
	SM1=0;			//设置串行口工作方式2
	TB8=0;			//发送数据时附加的第九位为TB8,
					//接收数据时SCON中的第九位为RB8;

	EA=1;			//开总中断
	ES=1;			//开串口中断
}

/*一般延时函数*/
void delayms(uchar xms)
{
	uchar i,j;
	for(i=xms;i>0;i--)
		for(j=114;j>0;j--);
}