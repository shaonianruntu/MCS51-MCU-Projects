#include<reg52.h>
#include<intrins.h>
#define uchar unsigned char
#define uint unsigned int
#define MAIN_Fosc	11059200UL	//宏定义主时钟HZ

sbit DS=P2^2;
sbit dula=P2^6;
sbit wela=P2^7;

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
void yj_display();	//液晶显示函数
void delayms(uint xms);		//ms级显示函数
void delayus(uchar xus);	//us级显示函数


unsigned long temp;
bit minus; //显示负号

/******************************************************
*******************************************************
******************************************************/
void main()
{
	uint i;
	uchar L,M;
	while(1)
	{
		DS_init();	 //初始化DS18B20
		DS_write_byte(0xcc);//跳过ROM
		DS_write_byte(0x44);//温度转换
		
		DS_init();//初始化DS18B20
		DS_write_byte(0xcc);//跳出ROM
		DS_write_byte(0xbe);//读暂存器
		L=DS_read_byte();
		M=DS_read_byte();
		/*模拟显示-25.0625度,因为没有DS18B20*/
		M=0xfe;
		L=0x6f;

		i = M;			//写入温度值高位
		i <<= 8;		//移到高位值到高位
		i |= L;			//补入低位值
		if(M >= 0x80)
		{
			i = ~i+1;  //负数是以补码的形式存放的,取反加1的原码
			minus=1;   //负数标记
		}
		else minus=0;	//正数
		temp = i *0.0625*10000;  //浮点型数据转换
		
		yj_display();		
	}
}


/******************************************************
*****************液晶显示函数**************************
******************************************************/
void yj_display()
{
	uchar i=0;
	/*第一位数码管*/
	if(minus == 0);				//如果为负数则不显示
	else
	{
		dula=0;
		P0=dutable[16];		//显示负数负号
		dula=1;
		dula=0;
		wela=0;
		P0=wetable[i++];
		wela=1;
		wela=0;
		delayms(1);
	}
	/*第二位数码管*/
	if(temp<1000000);	   //如果温度值百位为0则不显示
	else
	{
		dula=0;
		P0=dutable[temp/1000000];
		dula=1;
		dula=0;
		wela=0;
		P0=wetable[i++];
		wela=1;
		wela=0;
		delayms(1);
	}
	/*第三位数码管*/
	if(temp<100000);	 //如果温度值十位为0则不显示
	else
	{
		dula=0;
		P0=dutable[temp%1000000/100000];
		dula=1;
		dula=0;
		wela=0;
		P0=wetable[i++];
		wela=1;
		wela=0;
		delayms(1);
	}
	/*第四位数码管*/   
	dula=0;
	P0=dutable[temp%100000/10000]|0x80; //个位后显示小数点
	dula=1;
	dula=0;
	wela=0;
	P0=wetable[i++];
	wela=1;
	wela=0;
	delayms(1);
	/*第五次数码管*/
	dula=0;
	P0=dutable[temp%10000/1000];
	dula=1;
	dula=0;
	wela=0;
	P0=wetable[i++];
	wela=1;
	wela=0;
	delayms(1);
	/*第六位数码管*/
	if((temp%100/10)==0&&(temp%1000/100)==0);
	else
	{
		dula=0;
		P0=dutable[temp%1000/100];
		dula=1;
		dula=0;
		wela=0;
		P0=wetable[i++];
		wela=1;
		wela=0;
		delayms(1);
	}
	/*第七位数码管*/
	if((temp%10)==0&&(temp%100/10)==0);
	else
	{
		dula=0;
		P0=dutable[temp%100/10];
		dula=1;
		dula=0;
		wela=0;
		P0=wetable[i++];
		wela=1;
		wela=0;
		delayms(1);
	}
	/*第八位数码管*/
	if((temp%10)==0);
	else
	{
	 	dula=0;
		P0=dutable[temp%10];
		dula=1;
		dula=0;
		wela=0;
		P0=wetable[i++];
		wela=1;
		wela=0;
		delayms(1);
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
	delayus(75);	//拉低总线661.95us,挂接在总线上的DS18B20将会全部复位
	DS=1;			//释放总线
	delayus(4);		//延时44.45us,等待DS18B20发回存在信号
	i=DS;
	delayus(20);	//延时434.35us
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