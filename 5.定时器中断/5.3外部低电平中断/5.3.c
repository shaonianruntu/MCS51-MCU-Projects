#include<reg52.h>

void init();

void main()
{
	init();
	while(1)
	{
		P1=0xff;	//熄灭所有的灯
	}
}

/*中断服务特殊功能寄存器配置*/
void init()
{
	EX0=1;			//开外部中断0,接口P3^2
	IT0=0;			//低电平触变方式
	EA=1;			//开总中断
}

/*中断服务程序*/
void int0() interrupt 0
{
	P1=0;
}