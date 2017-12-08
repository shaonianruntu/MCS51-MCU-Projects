#include<reg52.h>
#include<stdio.h>		    //printf函数头文件

#define uchar unsigned char 
#define uint unsigned int

void init();
void delayms(uint xms);

void main()
{
	init();
	while(1)
	{
		TI=1;				 //printf函数使用前，需要将TI置1
		printf("杭州电子科技大学欢迎你!\n");
		delayms(100);		 //延时，使发送数据容易看到
		puts("电子信息工程学院欢迎你！");
		while(!TI);
		TI=0;
		delayms(2000);		 //延时，使发送数据容易看到
	}
}

/*串口初始化*/
void init()
{
	TMOD=0x20;			 //T1工作模式为2 8位自动重装
	TH1=253;			 
	TL1=253;			 //比特率9600
	TR1=1;				 //启动T1定时器

	SM0=0;
	SM1=1;				 //串口工作方式1 10位异步
}		 

/*延时函数*/
void delayms(uint xms)
{
	uint i,j;
	for(i=xms;i>0;i--)
		for(j=114;j>0;j--);
}