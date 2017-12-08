#include<reg52.h>
#include<intrins.h>
#define uchar unsigned char
#define uint unsigned int
#define AT24C02_ADD 0xa0            //AT24C02地址

sbit dula=P2^6;						//段选
sbit wela=P2^7;						//位选
sbit SDA=P2^0;						//时钟总线
sbit SCL=P2^1;						//数据总线

uchar code dutable[]={
0x3f,0x06,0x5b,0x4f,
0x66,0x6d,0x7d,0x07,
0x7f,0x6f,0x77,0x7c,
0x39,0x5e,0x79,0x71};

void I2C_init();
void init();
void delay();
void I2C_start();
void I2C_stop();
void I2C_response();
void write_byte(uchar date);
uchar read_byte();
void write_add(uchar address,uchar date);
uchar read_add(uchar address);
void display(uchar shi,uchar ge);
void delayms(uchar xms);

uchar sec,tcnt;
bit write=0;		//写24CO2的标志

void main()
{
	I2C_init();
	sec=read_add(2);				//读出保存的数据赋于sec
	if(sec>100)						//防止首次读取出错误数据
		sec=0;
	init();
	while(1)
	{
		display(sec/10,sec%10);
		if(1==write)				//判断T0是否计时了1秒
		{
			write=0;				//清零
			write_add(2,sec);		//在24C02的地址2中写入数据sec	
		}
	}	
}

/*T0定时器定时中断服务函数*/
void T0_time() interrupt 1
{
	TH0=(65536-45872)/256;
	TL0=(65536-46872)%256;			 //50ms
	tcnt++;
	if(20==tcnt)					 //1s
	{
		tcnt=0;
		sec++;						 
		write=1;					 //1秒写一次24C02
		if(100==sec)
			sec=0;		
	}
}

/*I2C初始化*/
void I2C_init()			//将总线都拉高,以释放总线	   
{
	SDA=1;
	delay();
	SCL=1;					   
	delay();
}

/*中断特殊服务寄存器初始化*/
void init()
{
	TMOD=0x01;				   //定时器0工作方式为1
	TH0=(65536-45872)/256;
	TL0=(65536-45872)%256;	   //50ms
	ET0=1;					   //开定时器0中断
	TR0=1;					   //启动定时器0中断
	EA=1;					   //开总中断
}

/*5us短暂延时*/
void delay()
{
	_nop_();				   //一个机器周期的延时
}

/*I2C开始信号*/
void I2C_start()		 //SCL在高电平期间,SDA一个下降沿启动信号	  
{
	SDA=1;
	delay();
	SCL=1;
	delay();
	SDA=0;
	delay();
}

/*I2C停止信号*/
void I2C_stop()		    //SCL在高电平期间，SDA一个上升沿停止信号
{
	SDA=0;
	delay();
	SCL=1;
	delay();
	SDA=1;
	delay();
}

/*I2C应答信号*/
void I2C_response()		//SCL在高电平期间，SDA被从设备拉为低电平表示应答
{
	uchar i;
	SCL=1;
	delay();
	while((1==SDA)&&(i<255))
		i++;
	SCL=0;
	delay();
}

/*写字节*/
void write_byte(uchar date)
{
	uchar i,temp;
	temp=date;
	for(i=0;i<8;i++)
	{
		temp=temp<<1;		  //将temp左移一位，最高位将移到PSW寄存器的CY位中
		SCL=0;				  //SCL为低电平时SDA才允许变化
		delay();			  //让总线稳定
		SDA=CY;				  //将CY赋值给SDA进而在SCL的控制下发送出去
		delay();			  //让总线稳定
		SCL=1;				  //SCL为高电平，让从机从SDA中把主机的数据读走
		delay();
	}
	SCL=0;					  //拉低SCL，占用总线继续通信
	delay();
	SDA=1;					  //释放SDA
	delay();
}

/*读字节*/
uchar read_byte()
{
	uchar i,k;
	SCL=0;
	delay();
	SDA=1;
	delay();
	for(i=0;i<8;i++)
	{
		SCL=1;
		delay();
		k=(k<<1)|SDA;		//将k左移一位后与SDA进行或运算，依次把8个独立的位放入一个字节中来完成接收
		SCL=0;
		delay();
	}
	return k;
}

/*写数据*/
void write_add(uchar address,uchar date)
{
	I2C_start();				    //启动
	write_byte(AT24C02_ADD);		//控制字 写	   
	I2C_response();					//应答
	write_byte(address);			//存储单元地址
	I2C_response();					//应答
	write_byte(date);				//数据
	I2C_response();					//应答
	I2C_stop();						//停止
}

/*读数据*/
uchar read_add(uchar address)
{
	uchar date;
	I2C_start();				   //启动
	write_byte(AT24C02_ADD+0);	   //控制字 写
	I2C_response();				   //应答
	write_byte(address);		   //单元地址
	I2C_response();				   //应答
	I2C_start();				   //启动
	write_byte(AT24C02_ADD+1);	   //控制字 读
	I2C_response();				   //应答
	date=read_byte();			   //数据
	I2C_stop();					   //停止
	return date;
}

/*数码管显示*/
void display(uchar shi,uchar ge)
{
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

/*延时*/
void delayms(uchar xms)
{
	uchar i,j;
	for(i=xms;i>0;i--)
		for(j=114;j>0;j--);
}