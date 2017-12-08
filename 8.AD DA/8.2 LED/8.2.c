#include<reg52.h>    		//包含头文件，一般情况不需要改动，头文件包含特殊功能寄存器的定义
#include<intrins.h> 		//包含NOP空指令函数_nop_();

#define uchar unsigned char 
#define uint unsigned int
#define PCF8591_ADD 0x90   //PCF8591地址 

sbit SDA=P2^0;     	  	  //i2c数据总线
sbit SCL=P2^1;		      //i2c时钟总线

void delayms(uchar xms);  				//一般延时函数
void init();							//程序初始化
void i2c_start();						//i2c启动函数
void i2c_stop();						//i2c停止函数
void DAC_ACK();						    //DAC的i2c应答信号
void i2c_sendbyte(uint bt);				//发送一字节数据
void DAC(uchar);						//DA数模转换函数


void main()
{
	uchar num;            //DA数模输出变量
    init();				  //函数初始化
    while(1)
    { 
       DAC(num);          //DA输出，用LED模拟电压变化
       num++;             //累加，到256后溢出变为0，往复循环。显示在LED上亮度逐渐变化
       delayms(20);       //延时用于清晰看出变化
    }
}

/*一般延时*/
void delayms(uchar xms)
{
  uint i,j;
  for(i=xms;i>0;i--)
     for(j=114;j>0;j--);
}

/*初始化*/
void init()
{
    TMOD = 0x20;        // 定时器1工作在方式2（自动重装）
    TH1 = 253;
    TL1 = 253;			//9600比特率
	TR1 = 1;			//启动T1定时器

	SM0=0;
	SM1=1;				//串口工作方式1, 10位异步
}

/*启动i2c,起始信号*/
void i2c_start()		//SCL在高电平期间,SDA一个下降沿启动信号	  
{
	SCL=1;
	_nop_();
	SDA=1;
	_nop_();
    SDA=0;
	_nop_();
	SCL=0;
	_nop_();
}

/*停止i2c*/
void i2c_stop()			//SCL在高电平的时候,SDA一个上升沿停止信号
{
	SCL=0;
	_nop_();
	SDA=0;
	_nop_();						
	SCL=1;
	_nop_();
	SDA=1;
	_nop_();	
}

/*i2c应答信号*/
void DAC_ACK()		   //SCL在高电平期间,SDA被从设备拉为低电平表示应答
{
	SDA=1;
	_nop_();
	SCL=1;
	_nop_();
	SCL=0;		
}

/*将输入的一字节数据bt发送*/
void i2c_sendbyte(uint bt)
{
	uint i;
	for(i=0;i<8;i++)
	{
		if(bt&0x80)
			SDA=1;
		else 
			SDA=0;
		_nop_();
		SCL=1;						//SCL为高电平,让从机从SDA中把主机的数据读走
		bt<<=1;						//将bt左移一位
		_nop_();
		SCL=0;						//拉低SCL,占用总线继续通信
	}
}

/*写入DA数模转换值*/
void DAC(uchar date)
{
	i2c_start();					//启动
	i2c_sendbyte(PCF8591_ADD+0);	//控制字,写入芯片地址   
    DAC_ACK();		   	    		//ACK应答
	i2c_sendbyte(0x40);	     		//写入控制位，使能DAC输出
    DAC_ACK();		   	 			//ACK应答             
    i2c_sendbyte(date);  			//写数据
    DAC_ACK();               		//ACK应答
    i2c_stop();                     //停止           
}