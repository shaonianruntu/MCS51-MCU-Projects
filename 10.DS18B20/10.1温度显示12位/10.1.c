#include<reg52.h>
#include<intrins.h>
#define uint unsigned int
#define uchar unsigned  char
#define MAIN_Fosc	11059200UL	//宏定义主时钟HZ

sbit dula=P2^6;	  //数码管段选
sbit wela=P2^7;	  //数码管位选
sbit DS=P2^2;	  //DS18B20单总线(信号线)

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

uchar code wetable[]={		//数码管位选码
//第1位 2位	 3位   4位       
0xfe, 0xfd, 0xfb, 0xf7, 
//5位  6位	 7位   8位
0xef, 0xdf, 0xbf, 0x7f};

bit DS_init();	  //单总线初始化函数
uchar DS_read_byte();	//DS18B20读数据
void DS_write_byte(uchar date);	//DS18B20写数据
void yj_display(uint num);	//液晶显示函数
void delayms(uint xms);		//ms级显示函数
void delayus(uchar xus);	//us级显示函数

/*******************************************************
********************************************************
*******************************************************/
void main()
{
	uint i;
	uchar L,M;	 //L:温度值低位,M:温度值高位
	while(1)
	{
		DS_init();	 //初始化DS18B20
		DS_write_byte(0xcc);//跳过ROM
		DS_write_byte(0x44);//温度转换
		
		DS_init();	 //初始化DS18B20
		DS_write_byte(0xcc);//跳过ROM
		DS_write_byte(0xbe);//读暂存器
		L=DS_read_byte();
		M=DS_read_byte();
		i = M;		 //写入温度值高位
		i <<= 8;	 //移到高位值到高位
		i |= L;		 //补入低位值
		i = i* 0.0625;//转换为十进制
		i=i*10+0.5;	//将小数点后一位数字移到小数点前,并四舍五入小数点后第二位
		yj_display(i);
	}

}


/*******************************************************
*********************DS18B20函数************************
*******************************************************/
/*单总线初始化函数*/
bit DS_init()
{
	bit i;
	DS=1;
	_nop_();
	DS=0;
	delayus(100);	//拉低总线661.95us,挂接在总线上的DS18B20将会全部复位
	DS=1;			//释放总线
	delayus(5);		//延时44.45us,等待DS18B20发回存在信号
	i=DS;
	delayus(65);	//延时434.35us
	DS=1;
	_nop_();
	return(i);
}

/*DS18B20写数据*/
void DS_write_byte(uchar date)
{
	uchar i;
	for(i=0;i<8;i++)
	{
		DS=0;
		_nop_();	//产生写时序
		DS=date&0x01;//按从低位到高位的顺序发送数据
		delayus(10);//延时76.95us
		DS=1;//释放总线准备下一次数据写入
		_nop_();
		date >>= 1;//左移准备传入倒数第二位
	}
}

/*DS18B20读数据*/
uchar DS_read_byte()
{
	uchar i,j,date;
	for(i=0;i<8;i++)
	{
		DS=0;	
		_nop_();	//产生读时序
		DS=1;
		_nop_();	//释放总线
		j=DS;	//得到最低位
		delayus(10);//延时76.95us
		DS=1;
		_nop_();
		date=(j<<7)|(date>>1);//将新获得的位移到最高位,将原来的最高位下移一位
	}
	return (date);
}


/*******************************************************
********************液晶显示函数************************
*******************************************************/
/*液晶显示*/
void yj_display(uint num)
{
	dula=0;						 
	P0=dutable[num/100];		 //十位数
	dula=1;
	dula=0;
	wela=0;
	P0=0xfe;
	wela=1;
	wela=0;
	delayms(5);

	dula=0;
	P0=dutable[num%100/10]|0x80; //个位数显示加小数点
	dula=1;						 //0x80表示小数点
	dula=0;
	wela=0;
	P0=0xfd;
	wela=1;
	wela=0;
	delayms(5);

	dula=0;
	P0=dutable[num%10];			 //小数位
	dula=1;
	dula=0;
	wela=0;
	P0=0xfb;
	wela=1;
	wela=0;
	delayms(5);
}


/*******************************************************
*********************延时函数***************************
*******************************************************/
/*ms级延时函数*/
void delayms(uint xms)
{
	uint i,j;
	for(i=xms;i>0;i--)
		for(j=114;j>0;j--);
}

/*us级延时函数,*/
void delayus(uchar xus)	//执行一次6.5us,进入一次11.95us
{
	while(xus--);
}