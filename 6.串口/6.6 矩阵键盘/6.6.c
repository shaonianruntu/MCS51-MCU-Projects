/*把矩阵键盘的键值以2400bps上传到计算机串口助手*/
#include<reg52.h>
#define uchar unsigned char
#define uint unsigned int

void init();
uchar KeyPro();
uchar KeyScan();
void delayms(uchar xms);

void main()
{
	init();		     	 //串口初始化
	while(1)
	{
		SBUF=KeyPro();	 //调用带返回值的键值码转换函数，把转换后的键值码送入发送SBUF
		while(!TI);		 //等待发送完毕
		TI=0;			 //清零
	}
}

void init()
{
	TMOD=0x20;		   //T1工作方式2 8位自动重装
	TH1=244;		   
	TH1=244;		   //比特率2400；256-11059200/2400/12/32
	TR1=1;			   //启动定时器T1
	
	SM0=0;			  
	SM1=1;			   //设置串口的工作方式1，10位异步
}

uchar KeyPro()
{
	uchar key;
	switch(KeyScan())
	{
		//第一行键值码
		case 0xee: key=0x01;	break;
		case 0xde: key=0x02;	break;
		case 0xbe: key=0x03;	break;
		case 0x7e: key=0x04;	break;

		//第二行键值码
		case 0xed: key=0x05;	break;
		case 0xdd: key=0x06;	break;
		case 0xbd: key=0x07;	break;
		case 0x7d: key=0x08;	break;

		//第三行键值码
		case 0xeb: key=0x09;	break;
		case 0xdb: key=0x0a;	break;
		case 0xbb: key=0x0b;	break;
		case 0x7b: key=0x0c;	break;

		//第四行键值码
		case 0xe7: key=0x0d;	break;
		case 0xd7: key=0x0e;	break;
		case 0xb7: key=0x0f;	break;
		case 0x77: key=0x10;	break;
	}
	return key;
}

uchar KeyScan()
{
	uchar lie,hang;		   //声明列线和行线的储存变量
	P3=0xf0;			   //1111 0000
	if((P3&0xf0)!=0xf0)	   //判断是否有按键按下
	{
		delayms(5);		   //软件消影
		if((P3&0xf0)!=0xf0) //判断是否有按键按下
		{
			lie=P3&0xf0;	 			//储存列线值
			P3=lie|0x0f;
			hang=P3&0x0f;				//储存行线值
			while((P3&0x0f)!=0x0f);		//松手检测
			return(lie+hang);			//返回键值码
		}
	}
}

void delayms(uchar xms)
{
	uchar i,j;
	for(i=xms;i>0;i--)
		for(j=114;j>0;j--);
}