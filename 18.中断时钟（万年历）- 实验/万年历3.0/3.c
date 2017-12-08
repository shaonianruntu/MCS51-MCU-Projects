//晶振：TX-1C实验板上实际晶振为11.0592M，
//这里为了方便计算时间,我们假设晶振为12M.
#include<reg52.h>//包含52单片机头文件
#define uchar unsigned char 
#define uint unsigned int
bit  write=0;           //写24C02的标志；
sbit sda=P2^0;                         
sbit scl=P2^1;
sbit dula=P2^6; //定义锁存器锁存端
sbit wela=P2^7;
sbit rs=P3^5;  //定义1602液晶RS端
sbit lcden=P3^4;//定义1602液晶LCDEN端
sbit s1=P3^0;   //定义按键--功能键
sbit s2=P3^1;	//定义按键--增加键
sbit s3=P3^2;	//定义按键--减小键
sbit rd=P3^7;
sbit beep=P2^3; //定义蜂鸣器端
uchar count,s1num;
char miao,shi,fen;
uchar code table[]="  2008-9-30 MON";//定义初始上电时液晶默认显示状态
void delay0()
{ ;; }
void start()  //开始信号
{	
	sda=1;
	delay0();
	scl=1;
	delay0();
	sda=0;
	delay0();
}
void stop()   //停止
{
	sda=0;
	delay0();
	scl=1;
	delay0();
	sda=1;
	delay0();
}
void respons()  //应答
{
	uchar i;
	scl=1;
	delay0();
	while((sda==1)&&(i<250))i++;
	scl=0;
	delay0();
}
void init_24c02()//IIC初始化函数
{
	sda=1;
	delay0();
	scl=1;
	delay0();
}
void write_byte(uchar date)//写一个字节函数
{
	uchar i,temp;
	temp=date;
	for(i=0;i<8;i++)
	{
		temp=temp<<1;
		scl=0;
	    delay0();
		sda=CY;
		delay0();
		scl=1;
		delay0();
	}
	scl=0;
	delay0();
	sda=1;
	delay0();
}
uchar read_byte()//读一个字节函数
{
	uchar i,k;
	scl=0;
	delay0();
	sda=1;
	delay0();
	for(i=0;i<8;i++)
	{
		scl=1;
		delay0();	
		k=(k<<1)|sda;
		scl=0;
		delay0();	
	}
	return k;
}
void write_add(uchar address,uchar date)//指定地址写一个字节
{
	start();
	write_byte(0xa0);
	respons();
	write_byte(address);
	respons();
	write_byte(date);
	respons();
	stop();
}
char read_add(uchar address)//指定地址读一个字节
{
	uchar date;
	start();
	write_byte(0xa0);
	respons();
	write_byte(address);
	respons();
	start();
	write_byte(0xa1);
	respons();
	date=read_byte();
	stop();
	return date;
}


void delay(uint z)  //延时函数
{
	uint x,y;
	for(x=z;x>0;x--)
		for(y=110;y>0;y--);
}
void di()//蜂鸣器发声函数
{
	beep=0;
	delay(100);
	beep=1;
}
void write_com(uchar com)//液晶写命令函数
{
	rs=0;
	lcden=0;
	P0=com;
	delay(5);
	lcden=1;
	delay(5);
	lcden=0;	
}
void write_date(uchar date)//液晶写数据函数
{
	rs=1;
	lcden=0;
	P0=date;
	delay(5);
	lcden=1;
	delay(5);
	lcden=0;	
}
void write_sfm(uchar add,uchar date)//写时分秒函数
{
	uchar shi,ge;
	shi=date/10;//分解一个2位数的十位和个位
	ge=date%10;
	write_com(0x80+0x40+add);//设置显示位置
	write_date(0x30+shi);//送去液晶显示十位
	write_date(0x30+ge); //送去液晶显示个位
}
void init()//初始化函数
{
	uchar num;
	rd=0;  //软件将矩阵按键第4列一端置低用以分解出独立按键
	dula=0;//关闭两锁存器锁存端，防止操作液晶时数码管会出乱码
	wela=0;
	lcden=0;
	fen=0; //初始化种变量值
	miao=0;
	shi=0;
	count=0;
	s1num=0;
	init_24c02();
	write_com(0x38);//初始化1602液晶
	write_com(0x0c);
	write_com(0x06);
	write_com(0x01);
	write_com(0x80);//设置显示初始坐标
	for(num=0;num<15;num++)//显示年月日星期
		{
			write_date(table[num]);
			delay(5);
		}
		write_com(0x80+0x40+6);//写出时间显示部分的两个冒号
		write_date(':');
		delay(5);
		write_com(0x80+0x40+9);
		write_date(':');
		delay(5);
	miao=read_add(1);//首次上电从AT24C02中读取出存储的数据
	fen=read_add(2);
	shi=read_add(3);
	write_sfm(10,miao);//分别送去液晶显示
	write_sfm(7,fen);
	write_sfm(4,shi);
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
		delay(5);
		if(s1==0)//确认功能键被按下
		{	s1num++;//功能键按下次数记录
			while(!s1);//释放确认
			di();//每当有按键释放蜂鸣器发出滴声
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
			delay(5);
			if(s2==0)//增加键确认被按下
			{
				while(!s2);//按键释放
				di();//每当有按键释放蜂鸣器发出滴声
				if(s1num==1)//若功能键第一次按下
				{
					miao++; //则调整秒加1
					if(miao==60)//若满60后将清零
						miao=0;
					write_sfm(10,miao);//每调节一次送液晶显示一下
					write_com(0x80+0x40+10);//显示位置重新回到调节处
					write_add(1,miao);	//数据改变立即存入24C02
				}
				if(s1num==2)//若功能键第二次按下
				{
					fen++;//则调整分钟加1
					if(fen==60)//若满60后将清零
						fen=0;
					write_sfm(7,fen);//每调节一次送液晶显示一下
					write_com(0x80+0x40+7);//显示位置重新回到调节处
					write_add(2,fen);//数据改变立即存入24C02
				}
				if(s1num==3)//若功能键第三次按下
				{
					shi++;//则调整小时加1
					if(shi==24)//若满24后将清零
						shi=0;
					write_sfm(4,shi);;//每调节一次送液晶显示一下
					write_com(0x80+0x40+4);//显示位置重新回到调节处
					write_add(3,shi);//数据改变立即存入24C02
				}
			}
		}
		if(s3==0)
		{
			delay(5);
			if(s3==0)//确认减小键被按下
			{
				while(!s3);//按键释放
				di();//每当有按键释放蜂鸣器发出滴声
				if(s1num==1)//若功能键第一次按下
				{
					miao--;//则调整秒减1
					if(miao==-1)//若减到负数则将其重新设置为59
						miao=59;
					write_sfm(10,miao);//每调节一次送液晶显示一下
					write_com(0x80+0x40+10);//显示位置重新回到调节处
					write_add(1,miao);//数据改变立即存入24C02
				}
				if(s1num==2)//若功能键第二次按下
				{
					fen--;//则调整分钟减1
					if(fen==-1)//若减到负数则将其重新设置为59
						fen=59;
					write_sfm(7,fen);//每调节一次送液晶显示一下
					write_com(0x80+0x40+7);//显示位置重新回到调节处
					write_add(2,fen);//数据改变立即存入24C02
				}
				if(s1num==3)//若功能键第二次按下
				{
					shi--;//则调整小时减1
					if(shi==-1)//若减到负数则将其重新设置为23
						shi=23;
					write_sfm(4,shi);//每调节一次送液晶显示一下
					write_com(0x80+0x40+4);//显示位置重新回到调节处
					write_add(3,shi);//数据改变立即存入24C02
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
				write_sfm(4,shi);//小时若变化则重新写入
				write_add(3,shi);//数据改变立即存入24C02
			}
			write_sfm(7,fen);//分钟若变化则重新写入
			write_add(2,fen);//数据改变立即存入24C02
		}
		write_sfm(10,miao);//秒若变化则重新写入	
		write_add(1,miao);//数据改变立即存入24C02
	}	
}

