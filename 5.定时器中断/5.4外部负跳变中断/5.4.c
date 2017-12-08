#include<reg52.h>
#define uint unsigned int

void init();
void delayms(uint);

void main()
{
	init();
	while(1)
	{
		P1=0Xff;
	}
}                      

/*中断服务特殊功能寄存器*/
void init()
{
	EX1=1;			//开外部中断1，接口P3^3
	IT1=1;			//负跳变触发方式
	EA=1;			//开总中断
}

/*中断服务程序*/
void int1() interrupt 2
{
	P1=0;
	delayms(500);		//延时便于观察现象的改变
}

/*延时子程序*/
void delayms(uint xms)
{
	uint i,j;
	for(i=xms;i>0;i--)
		for(j=115;j>0;j--);
}