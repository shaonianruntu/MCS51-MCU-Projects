#include<reg52.h>
#include<intrins.h>
#define uchar unsigned char
#define uint unsigned int
#define MAIN_Fosc		11059200UL	//宏定义主时钟HZ

sbit dula=P2^6;
sbit wela=P2^7;
sbit DS=P2^2;

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

bit DS_init();	  //单总线初始化函数
uchar DS_read_byte();	//DS18B20读数据
void DS_write_byte(uchar date);	//DS18B20写数据
void yj_display(uint num);//液晶显示函数
void delayms(uint xms);	  //ms级延时函数
void delayus(uchar xus);  //us级延时函数


/*******************************************************
********************************************************
*******************************************************/
void main()
{
	uint i;
	uchar L,M;
	DS_init();//初始化DS18B20
	DS_write_byte(0xcc);//跳出ROM
	DS_write_byte(0x4e);//写寄存器
	DS_write_byte(0x30);//由上到下写入上限温度数据命令
	DS_write_byte(0x40);//下线温度数据命令
	DS_write_byte(0x1f);//设置工作在9位模式下(0001 1111)
	DS_init();//初始化DS18B20
	DS_write_byte(0xcc);//跳出ROM 
	DS_write_byte(0x48);//将更改后的2,3字节中的数据复制到E2PROM
	while(1)
	{
		DS_init();//初始化DS18B20
		DS_write_byte(0xcc);//跳出ROM
		DS_write_byte(0x44);//温度转换

		DS_init();//初始化DS18B20
		DS_write_byte(0xcc);//跳出ROM
		DS_write_byte(0xbe);//读暂存器
		L=DS_read_byte();
		M=DS_read_byte();
		i = M;			//写入温度值高位
		i <<= 8;		//移到高位值到高位
		i |= L;			//补入低位值
		i = i *0.0625;  //转换为十进制
		i = i *	10+0.5;	//将小数点后一位数字移到小数点前,并四舍五入小数点后第二位
		
		yj_display(i);
	}	
}


/******************************************************
*****************DS18B20******************************
******************************************************/
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


/******************************************************
*****************液晶显示函数**************************
******************************************************/
void yj_display(uint num)
{
	dula=0;
	P0=dutable[num/100];
	dula=1;
	dula=0;
	wela=1;
	P0=0xfe;
	wela=1;
	wela=0;
	delayms(5);

	dula=0;
	P0=dutable[num%100/10]|0x80;
	dula=1;
	dula=0;
	wela=0;
	P0=0xfd;
	wela=1;
	wela=0;
	delayms(5);

	dula=0;
	P0=dutable[num%10];
	dula=1;
	dula=0;
	wela=0;
	P0=0xfd;
	wela=1;
	wela=0;
	delayms(5);
}


/******************************************************
*****************延时函数******************************
******************************************************/
/*ms级延时函数*/
void delayms(uint xms)
{
	uint i,j;
	for(i=xms;i>0;i--)
		for(j=114;j>0;j--);
}

/*us级延时函数*/
void delayus(uchar xus)
{
	while(xus--);
}