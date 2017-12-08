//晶振：TX-1C实验板上实际晶振为11.0592M，
//这里为了方便计算时间,我们假设晶振为12M.
#include<reg52.h>//包含52单片机头文件
#define uchar unsigned char 
#define uint unsigned int
#define AT24C02_ADD 0xa0            //AT24C02地址

bit  write=0;           //写24C02的标志；

sbit SDA=P2^0;						//时钟总线
sbit SCL=P2^1;						//数据总线

sbit dula=P2^6; //定义锁存器锁存端
sbit wela=P2^7;

sbit lcdrs=P3^5;		   //RS液晶数据命令选择端
sbit lcdrw=P3^6;		   //WR液晶读写选择端
sbit lcden=P3^4;		   //LCDEN液晶使能端

sbit s1=P3^0;   //定义按键--功能键
sbit s2=P3^1;	//定义按键--增加键
sbit s3=P3^2;	//定义按键--减小键
sbit s4=P3^3;			   //模式切换键

sbit beep=P2^3; //定义蜂鸣器端

uchar count,s1num;
char miao,shi,fen;
uchar code table[]="  WORK CLOCK";//定义初始上电时液晶默认显示状态

void delay()
{ ;; }

/*启动i2c,起始信号*/
void i2c_start()		//SCL在高电平期间,SDA一个下降沿启动信号	  
{
	SCL=1;
	delay();
	SDA=1;
	delay();
    SDA=0;
	delay();
	SCL=0;
	delay();
}

/*停止i2c*/
void i2c_stop()			//SCL在高电平的时候,SDA一个上升沿停止信号
{
	SCL=0;
	delay();
	SDA=0;
	delay();						
	SCL=1;
	delay();
	SDA=1;
	delay();	
}

/*i2c应答信号*/
void i2c_respons()  				//SCL在高电平期间,SDA被从设备拉为低电平表示应答
{
	uchar i;
	SDA=0;
	delay();
	SCL=1;
	delay();
	while((SDA==1)&&(i<250))
		i++;
	SCL=0;
	delay();
	SDA=1;
	delay();
}

/*将输入的一字节数据bt发送*/
void i2c_sendbyte(uchar bt)
{
	uchar i;
	for(i=0;i<8;i++)
	{
		if(bt&0x80)
			SDA=1;
		else 
			SDA=0;
		delay();
		SCL=1;						//SCL为高电平,让从机从SDA中把主机的数据读走
		bt<<=1;						//将bt左移一位
		delay();
		SCL=0;						//拉低SCL,占用总线继续通信
	}
}

/*从总线上接收1字节数据*/
uchar i2c_recbyte()
{
	uchar date,i;
	for(i=0;i<8;i++)
	{
		SCL=1;						//SCL为高电平,让从机从SDA中把主机的数据读走
		delay();					 
		date<<=1;					//左移进位,对下一位数字进行操作
		if(SDA)
		{
			date=date|0x01;		   	//赋值,接收到为1,没有为0
		}
		SCL=0;						//拉低SCL,占用总线继续通信
		delay();
	}
	return date;
}

/*指定地址读出一字节数据*/
char i2c_read(uchar address)
{
	uchar date;	

	i2c_start();					 //启动
	i2c_sendbyte(AT24C02_ADD+0);	 //控制字 写
	i2c_respons();					 //应答
	i2c_sendbyte(address);			 //存储单元地址
	i2c_respons();					 //应答
	i2c_start();					 //启动
	i2c_sendbyte(AT24C02_ADD+1);	 //控制字 读
	i2c_respons();					 //应答
	date=i2c_recbyte();				 //接收数据
	i2c_stop();						 //停止
	return date;
} 

/*指定地址写入一字节数据*/
void i2c_write(uchar address, char dat)
{
	i2c_start();					 //启动
	i2c_sendbyte(AT24C02_ADD+0);	 //控制字 写
	i2c_respons();					 //应答
	i2c_sendbyte(address);			 //存储单元地址
	i2c_respons();					 //应答
	i2c_sendbyte(dat);	 			 //数据
	i2c_respons();					 //应答
	i2c_stop();						 //停止
} 

/*i2c初始化*/
void i2c_init()			//将总线都拉高,以释放总线	   
{
	SDA=1;
	delay();
	SCL=1;					   
	delay();
}

/*一般延时函数*/
void delayms(uchar xms)
{
	uchar i,j;
	for (i=xms;i>0;i--)
		for (j=114;j>0;j--);   //1ms
}

/*蜂鸣器函数*/
void beeper()
{
	beep=0;				//开启蜂鸣器
	delayms(100);		//0.1s延时
	beep=1;				//关闭蜂鸣器
}

/*液晶命令写入*/
void write_com (uchar com)
{
	lcdrw=0;		   //选择写
	lcdrs=0;		   //选择命令
	P0=com;
	delayms(5);		   //延时使液晶运行稳定
	lcden=1;		   //高脉冲将数据送入液晶控制器
	delayms(5);
	lcden=0;
}

/*液晶数据写入*/
void write_data(uchar date)
{
	lcdrw=0;		  //选择写
	lcdrs=1;		  //选择数据
	P0=date;		  
	delayms (5);	  //延时使液晶运行稳定
	lcden=1;		  //高脉冲将数据送入液晶控制器
	delayms (5);
	lcden=0;
}

/*液晶显示函数*/
void display(uchar add,uchar date)//显示时分秒函数。add什么位置 date显示什么
{
	uchar shi,ge;
	shi=date/10;	
	ge=date%10;

	write_com(add);
	write_data(shi+'0');
	write_data(ge+'0');
}


void init()//初始化函数
{
	uchar num;
	dula=0;//关闭两锁存器锁存端，防止操作液晶时数码管会出乱码
	wela=0;
	lcden=0;
	fen=0; //初始化种变量值
	miao=0;
	shi=0;
	count=0;
	s1num=0;
	i2c_init();
	write_com(0x38);//初始化1602液晶
	write_com(0x0c);
	write_com(0x06);
	write_com(0x01);
	write_com(0x80);//设置显示初始坐标
	for(num=0;num<15;num++)//显示年月日星期
		{
			write_data(table[num]);
			delayms(5);
		}
		write_com(0x80+0x40+6);//写出时间显示部分的两个冒号
		write_data(':');
		delayms(5);
		write_com(0x80+0x40+9);
		write_data(':');
		delayms(5);
	miao=i2c_read(1);//首次上电从AT24C02中读取出存储的数据
	fen=i2c_read(2);
	shi=i2c_read(3);
	display(0x80+0x40+10,miao);//分别送去液晶显示
	display(0x80+0x40+7,fen);
	display(0x80+0x40+4,shi);
	TMOD=0x01;//设置定时器0工作模式1
	TH0=(65536-50000)/256;//定时器装初值
	TL0=(65536-50000)%256;
	EA=1;	//开总中断
	ET0=1;	//开定时器0中断
	TR0=1;	//启动定时器0
}
void keyscan()//按键扫描函数
{
	if(s1==0)	
	{
		delayms(5);
		if(s1==0)//确认功能键被按下
		{	s1num++;//功能键按下次数记录
			while(!s1);//释放确认
			beeper();//每当有按键释放蜂鸣器发出滴声
			if(s1num==1)//第一次被按下时
			{
				TR0=0;  //关闭定时器
				write_com(0x80+0x40+10);//光标定位到秒位置
				write_com(0x0f); //光标开始闪烁
			}
			if(s1num==2)//第二次按下光标闪烁定位到分钟位置
			{
				write_com(0x80+0x40+7);
			}
			if(s1num==3)//第三次按下光标闪烁定位到小时位置
			{
				write_com(0x80+0x40+4);
			}
			if(s1num==4)//第四次按下
			{
				s1num=0;//记录按键数清零
				write_com(0x0c);//取消光标闪烁
				TR0=1;	//启动定时器使时钟开始走
			}		
		}
	}
	if(s1num!=0)//只有功能键被按下后，增加和减小键才有效
	{
		if(s2==0)
		{
			delayms(5);
			if(s2==0)//增加键确认被按下
			{
				while(!s2);//按键释放
				beeper();//每当有按键释放蜂鸣器发出滴声
				if(s1num==1)//若功能键第一次按下
				{
					miao++; //则调整秒加1
					if(miao==60)//若满60后将清零
						miao=0;
					display(0x80+0x40+10,miao);//每调节一次送液晶显示一下
					i2c_write(0x80+0x40+10);//显示位置重新回到调节处
					i2c_write(1,miao);	//数据改变立即存入24C02
				}
				if(s1num==2)//若功能键第二次按下
				{
					fen++;//则调整分钟加1
					if(fen==60)//若满60后将清零
						fen=0;
					display(0x80+0x40+7,fen);//每调节一次送液晶显示一下
					write_com(0x80+0x40+7);//显示位置重新回到调节处
					i2c_write(2,fen);//数据改变立即存入24C02
				}
				if(s1num==3)//若功能键第三次按下
				{
					shi++;//则调整小时加1
					if(shi==24)//若满24后将清零
						shi=0;
					display(0x80+0x40+4,shi);;//每调节一次送液晶显示一下
					write_com(0x80+0x40+4);//显示位置重新回到调节处
					i2c_write(3,shi);//数据改变立即存入24C02
				}
			}
		}
		if(s3==0)
		{
			delayms(5);
			if(s3==0)//确认减小键被按下
			{
				while(!s3);//按键释放
				beeper();//每当有按键释放蜂鸣器发出滴声
				if(s1num==1)//若功能键第一次按下
				{
					miao--;//则调整秒减1
					if(miao==-1)//若减到负数则将其重新设置为59
						miao=59;
					display(0x80+0x40+10,miao);//每调节一次送液晶显示一下
					write_com(0x80+0x40+10);//显示位置重新回到调节处
					i2c_write(1,miao);//数据改变立即存入24C02
				}
				if(s1num==2)//若功能键第二次按下
				{
					fen--;//则调整分钟减1
					if(fen==-1)//若减到负数则将其重新设置为59
						fen=59;
					display(0x80+0x40+7,fen);//每调节一次送液晶显示一下
					write_com(0x80+0x40+7);//显示位置重新回到调节处
					i2c_write(2,fen);//数据改变立即存入24C02
				}
				if(s1num==3)//若功能键第二次按下
				{
					shi--;//则调整小时减1
					if(shi==-1)//若减到负数则将其重新设置为23
						shi=23;
					display(0x80+0x40+4,shi);//每调节一次送液晶显示一下
					write_com(0x80+0x40+4);//显示位置重新回到调节处
					i2c_write(3,shi);//数据改变立即存入24C02
				}
			}
		}
	}
}
void main()//主函数
{
	init();//首先初始化各数据
	while(1)//进入主程序大循环
	{
		keyscan();//不停的检测按键是否被按下
	}
}
void timer0() interrupt 1//定时器0中断服务程序
{
	TH0=(65536-50000)/256;//再次装定时器初值
	TL0=(65536-50000)%256;
	count++;		//中断次数累加
	if(count==20) //20次50毫秒为1秒
	{
		count=0;
		miao++;
		if(miao==60)//秒加到60则进位分钟
		{
			miao=0;//同时秒数清零
			fen++;
			if(fen==60)//分钟加到60则进位小时
			{
				fen=0;//同时分钟数清零
				shi++;
				if(shi==24)//小时加到24则小时清零
				{
					shi=0;
				}
				display(0x80+0x40+4,shi);//小时若变化则重新写入
				write_add(3,shi);//数据改变立即存入24C02
			}
			display(0x80+0x40+7,fen);//分钟若变化则重新写入
			write_add(2,fen);//数据改变立即存入24C02
		}
		display(0x80+0x40+10,miao);//秒若变化则重新写入	
		write_add(1,miao);//数据改变立即存入24C02
	}	
}
