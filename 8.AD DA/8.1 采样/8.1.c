#include<reg52.h>
#include<intrins.h>

#define uchar unsigned char
#define uint unsigned int
#define PCF8591_ADD 0x90	//PCF8591地址

sbit SDA=P2^0;			 	//i2c数据总线
sbit SCL=P2^1;				//i2c时钟总线

void delayms(uint xms);						  //一般延时函数
void i2c_start();							  //i2c启动函数
void i2c_stop();							  //i2c停止函数
void i2c_ACK(bit ck);						  //i2c应答信号
bit i2c_waitACK();							  //i2c应答接收判断信号
void i2c_sendbyte(uint bt);					  //发送一字节数据
uint i2c_recbyte();							  //接收一字节数据
bit i2c_readbyte(uint address, uint *dat);	  //读取一字节数据
void init(void);							  //程序初始化

void main()
{
	uint ans;								 //SBUF变量
	init();									 //程序初始化
	while(1)
	{
		i2c_readbyte(0x43,&ans);			 //读出保存的数据并赋值给ans
		SBUF = ans;							 
        while (!TI);						 //判断SBUF是否发送成功
        TI = 0;								 //清零
		delayms(1000);						 //1s延时
	}
}

/*一般延时*/
void delayms(uint xms)
{
	uint i,j;
	for(i=xms;i>0;i--)
		for(j=114;j>0;j--);
}

/*启动i2c,起始信号*/
void i2c_start()							 //SCL在高电平期间,SDA一个下降沿启动信号	  
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
void i2c_stop()								//SCL在高电平的时候,SDA一个上升沿停止信号
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
/*ck为1时放送应答信号,
  ck为0时不发送*/
void i2c_ACK(bit ck)					   //SCL在高电平期间,SDA被从设备拉为低电平表示应答
{
	if(ck)
		SDA=0;
	else 
		SDA=1;
	_nop_();
	SCL=1;
	_nop_();
	SCL=0;
	_nop_();
	SDA=1;
	_nop_();
}

/*i2c应答信号接收反馈信号*/
/*返回为1时接收到ACK
  返回为0时没有接收到ACK*/
bit i2c_waitACK()
{
	SDA=1;
	_nop_();
	SCL=1;
	_nop_();
	if(SDA)
	{
		SCL=0;
		i2c_stop();
		return 0;
	}
	else
	{
		SCL=0;
		return 1;
	}		
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

/*从总线上接收1字节数据*/
uint i2c_recbyte()
{
	uint date,i;
	for(i=0;i<8;i++)
	{
		SCL=1;						//SCL为高电平,让从机从SDA中把主机的数据读走
		_nop_();					 
		date<<=1;					//左移进位,对下一位数字进行操作
		if(SDA)
		{
			date=date|0x01;		   	//赋值,接收到为1,没有为0
		}
		SCL=0;						//拉低SCL,占用总线继续通信
		_nop_();
	}
	return date;
}

/*读出一字节数据*/
/*返回1成功,  
  返回0失败*/
bit i2c_readbyte(uint address, uint *dat)
{
	i2c_start();					 //启动
	i2c_sendbyte(PCF8591_ADD+0);	 //控制字 写
	if(!i2c_waitACK())				 //ACK应答
		return 0;
	i2c_sendbyte(address);			 //存储单元地址
	if(!i2c_waitACK())				 //ACK应答
		return 0;
	i2c_start();					 //启动
	i2c_sendbyte(PCF8591_ADD+1);	 //控制字 读
	if(!i2c_waitACK())				 //ACK应答
		return 0;
	*dat=i2c_recbyte();				 //接收数据
	i2c_ACK(0);						 //停止应答
	i2c_stop();						 //停止
	return 1;
}

/*初始化*/
void init(void)
{
    TMOD = 0x20;        // 定时器1工作在方式2（自动重装）
    TH1 = 253;
    TL1 = 253;			//9600比特率
	TR1 = 1;			//启动T1定时器

	SM0=0;
	SM1=1;				//串口工作方式1, 10位异步
}
