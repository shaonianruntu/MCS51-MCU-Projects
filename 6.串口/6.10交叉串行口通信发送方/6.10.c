#include<reg52.h>
#define uchar unsigned char 
#define uint unsigned int

void init();			//串口通信服务初始化函数
void matrixkeyscan();	//矩阵键盘扫描函数
void send(uchar);		//串行口通信输出函数
void delayms(uint);		//一般延时函数
			
void main()
{
	init();				//初始化
	while(1)
	{
		matrixkeyscan();	 //矩阵键盘扫描程序
	}
}		

/*串口通信服务初始化函数*/
void init()
{
	TMOD=0x20;		//设置T1定时器工作方式2,8位自动重新
	TH1=253;		//9600比特率
	TL1=253;
	TR1=1;			//启动定时器1
	
	SM0=0;			//设置串口工作方式1
	SM1=1;
	
	EA=1;			//开总中断
	ES=1;			//开串口中断
}

/*矩阵键盘扫描程序*/
void matrixkeyscan()
{
	uchar temp,key;			 //temp用于获取按键信息,key用于储存
	P3=0xfe;
	temp=P3;
	temp=temp&0xf0;
	if(temp!=0xf0)
	{
		delayms(10);		 //延时防抖
		temp=P3;			 
		temp=temp&0xf0;		 
		if(temp!=0xf0)		 //重新判断防抖,提高稳定度
		{
			temp=P3;		 //获取键盘上的值
			switch(temp)
			{
				case 0xee:	key=1;	break;
				case 0xde:	key=2;	break;
				case 0xbe:	key=3;	break;
				case 0x7e:	key=4;	break;
			}
			while(temp!=0xf0)  //等待按键释放
			{
				temp=P3;
				temp=temp&0xf0;
			}
			send(key);	   	   //送出到串口通信
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
				case 0xed:	key=5;	break;
				case 0xdd:	key=6;	break;
				case 0xbd:	key=7;	break;
				case 0x7d:	key=8;	break;
			}
			while(temp!=0xf0)
			{
				temp=P3;
				temp=temp&0xf0;
			}
			send(key);
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
				case 0xeb:	key=9;	break;
				case 0xdb:	key=10;	break;
				case 0xbb:	key=11;	break;
				case 0x7b:	key=12;	break;
			}
			while(temp!=0xf0)
			{
				temp=P3;
				temp=temp&0xf0;
			}
			send(key);
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
				case 0xe7:	key=13;	break;
				case 0xd7:	key=14;	break;
				case 0xb7:	key=15;	break;
				case 0x77:	key=16;	break;
			}
			while(temp!=0xf0)
			{
				temp=P3;
				temp=temp&0xf0;
			}
			send(key);
		}
	}	
}

/*一般延时函数*/
void delayms(uint xms)
{
	uint i,j;
	for(i=xms;i>0;i--)
		for(j=110;j>0;j--);
}

/*串口通信输出*/
void send(uchar num)
{
	SBUF=num;
	while(!TI);
	TI=0;
}
 