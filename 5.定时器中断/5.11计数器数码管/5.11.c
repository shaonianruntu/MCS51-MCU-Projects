/*
定时器0工作方式1,用杜邦线一段接GND,一段接T0(P3^4)引脚,
每接触一下,计数器计一次数,将所计的数字实时显示在数码管
的前两位,计满100清零
*/

/*导线在接触单片机引脚的时候会发生抖动,导致接触一次产生的数不为1*/

#include<reg52.h>
#define uint unsigned int
#define uchar unsigned char

uchar code dutable[]={		//共阴极数码管段选码
//0   1    2	3 
0x3f,0x06,0x5b,0x4f,
//4   5	   6 	7
0x66,0x6d,0x7d,0x07,
//8   9	   A	b
0x7f,0x6f,0x77,0x7c,
//C   d	   E	F
0x39,0x5e,0x79,0x71,
//-   .  关显示
0x40,0x80,0x00};

sbit dula=P2^6;
sbit wela=P2^7;

void init();		  //计数器初始化函数
uint read();		  //读取运行中计数器寄存器中的值
void display(uchar);  //数码管显示函数
void delayms(uchar);  //一般延时函数

void main()
{
	uint num;
	init();
	while(1)
	{
		num=read();
		if(100<=num)
		{
			num=0;
			TH0=0;	//将计数器的寄存器值清零
			TL0=0;
		}
		display(num);
	}
}

/*计数器初始化函数*/
void init()
{
	TMOD=0x05;		//设置计数器0的工作方式1(0000 0101)
	TH0=0;			//将计数器寄存器值清零
	TL0=0;
	TR0=1;			//开启计数器T0
}

/*读取运行中计数器寄存器中的值*/
uint read()
{
	uchar tl0,th01,th02;
	uint val;
	while(1)
	{
		th01=TH0;			  //寄存器的值会随时变化,只读一次,
		tl0=TL0;			  //当发生进位时,可能会读错数据,
		th02=TH0;			  //所以TH0的数据需要读两次,
		if(th01==th02)		  //以确保读取的时候没有发生进位
			break;
	} 
	val=th01*256+tl0;
	return val;
}

/*数码管显示函数*/
void display(uchar date)
{
	uchar shi,ge;
	shi=date/10;
	ge=date%10;

	dula=1;
	P0=dutable[shi];
	dula=0;
	P0=0xff;
	wela=1;
	P0=0xfe;
	wela=0;
	delayms(5);

	dula=1;
	P0=dutable[ge];
	dula=0;
	P0=0xff;
	wela=1;
	P0=0xfd;
	wela=0;
	delayms(5);
}

/*一般延时函数*/
void delayms(uchar xms)
{
	uchar i,j;
	for(i=xms;i>0;i--)
		for(j=1114;j>0;j--);
}
