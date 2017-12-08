/********************************************************************************
程序：用定时器编写电子时钟程序，设定三个功能键，一个功能键，一个数值增大键；一个数
值减小键来设定时分秒。并且能将数据存入到AT2404中，再次启动电源，电子表从断电时刻开始
继续计时
**********************************************************************************/
#include <reg52.h>
#define uchar unsigned char 
#define uint unsigned int 
sbit lcdrs=P3^5;		   //RS液晶数据命令选择端
sbit lcdrw=P3^6;		   //WR液晶读写选择端
sbit lcden=P3^4;		   //LCDEN液晶使能端
sbit dula=P2^6;			   //段选
sbit wela=P2^7;			   //位选
sbit beep=P2^3;			   //蜂鸣器

sbit s1=P3^0;			   //设为功能选择键
sbit s2=P3^2;  			   //设为数值增大键
sbit s3=P3^3;			   //设为数值减小键

uchar shuzi[10]="0123456789";
uchar table[14]="A SIMPLE CLOCK";

char hour=12 ,min=0,sec=0;//设置时分秒位

uchar time=0,addr;  //addr:光标选择位

void init();				   //定时器和1602显示屏初始化设定函数
void write_com(uchar);		   //液晶命令写入
void write_data(uchar);		   //液晶数据写入
void display(uchar,uchar);	   //液晶显示函数
void keyscan();				   //独立按键数字修改函数
void beeper();				   //蜂鸣器函数
void delayms(uchar);				   //一般延时函数

void main ()
{
	uchar i;
	init ();
	write_com (0x80+0x01);
	for (i=0;i<14;i++)
	{
		write_data(table[i]);	
	}
	display(0x80+0x44,hour);
	write_data(':');
	display(0x80+0x47,min);
	write_data(':');
	display(0x80+0x4a,sec);
	while (1)
	{
		keyscan ();	
	}
}

/*中断服务*/
void T0_time() interrupt 1
{
	TH0=(65536-45872)/256;		  //50ms,重装初值
	TL0=(65536-45872)%256;
	time++;
	if (time==20)				  //1s
	{
		time=0;					  //定时器计数归零
		sec++;					  //表针增加一位
		if (sec==60)			  //1min
		{
			sec=0;
			min++;
			if (min==60)		  //1h
				{
					min=0;
					hour++;
					if (hour==24)
						{hour=0;}
					display(0x80+0x44,hour);//时变化则重新写入时
				}
			display(0x80+0x47,min);//分变化则重新写入分
		}
		display(0x80+0x4a,sec);//秒变化则重新写入秒
	}
}

/*定时器和1602显示屏初始化设定函数*/
void init ()
{
	dula=0;		 	
	wela=0;			//关闭段选和位选,防止数码管点亮
	lcden=0;		//关闭液晶使能端

	TMOD=0X01;					  //设置定时器0工作方式1
	TH0=(65536-45872)/256;		  //50ms
	TL0=(65536-45872)%256;
	EA=1;						  //开总中断
	ET0=1;						  //开定时器0中断
	TR0=1;						  //启动定时器0
	
	write_com(0x38);		 //00111000B,设置为16*2显示，5*7点阵，8位数据接口
	write_com(0x0c);		 //00001100B,设置开显示，不显示光标
	write_com(0x06);		 //00000110B,写一个字符后地址指针加1
	write_com(0x01);		 //00000001B,显示清0，数据指针清0
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

/*一般延时函数*/
void delayms(uchar xms)
{
	uchar i,j;
	for (i=xms;i>0;i--)
		for (j=114;j>0;j--);

}

/*独立按键数字修改函数*/
void keyscan ()
{
	if (s1==0)
		{
			delayms (10);		   //延时防抖
			if (s1==0)			   //重复判断,防抖
				{
					addr++;		   //液晶选择位变化
					beeper();	   //按键按下时蜂鸣器响0.1s,作为反馈
					while (!s1);   //等待按键恢复
										
					if (addr==1)
						{
							TR0=0;
							write_com(0x80+0x4b);	 //光标移到秒位置(第二排12位)
							write_com(0x0f);		 //光标闪烁
						}
					if (addr==2)
						{
							write_com(0x80+0x48);	 //光标移到分位置(第二排9位)
							write_com(0x0f);		 //光标闪烁
						}
					if (addr==3)					 
						{
							write_com(0x80+0x45);	  //光标移到时位置(第二排6位)
							write_com(0x0f);		  //光标闪烁
						}
					if (addr==4)
						{
							addr=0;				  	  //光标选择位清零(取消选择)
							write_com(0x0c);		  //开显示,但取消光标闪烁
							TR0=1;					  //启动时钟,表开始走
						}
				}
		}
	if (s2==0&&addr!=0)//只有功能键被按下之后数值键才起作用
		{
			delayms (10);			//延时防抖
			if (s2==0)				//重复判断,防抖
				{	
					beeper ();		//按键按下时蜂鸣器响,作为反馈
					while (!s2);	//等待按键恢复
					if (addr==1)	//光标选择位在秒位
 						{
							sec++;			 
							if (sec==60)
								sec=0;
							display(0x80+0x4a,sec);//秒变化时重新写入
							write_com(0x80+0x4b);//写入后将光标移回并闪烁，显示为调整状态
						}
					if (addr==2) 	 //光标选择位在分钟位
						{
							min++;
							if (min==60)
								min=0;	
							display(0x80+0x47,min);//分变化时重新写入
							write_com(0x80+0x48);//写入后将光标移回并闪烁，显示调整状态
						}
					if (addr==3) 	 //光标选择位在小时位
						{
							hour++;
							if (hour==24)
								hour=0;
							display(0x80+0x44,hour);//时变化时重新写入	
							write_com(0x80+0x45);//写入后将光标移回并闪烁，显示调整状态
						}
				}
		}

	if (s3==0&&addr!=0)//只有功能键被按下之后数值键才起作用
		{
			delayms (10);		   //延时防抖
			if (s3==0)			   //重复判断,防抖
				{	
					beeper ();	   //按键按下时蜂鸣器响,作为反馈
					while (!s3);   //等待按键恢复
					if (addr==1)
 						{
							sec--;
							if (sec==-1)
								sec=59;
							display(0x80+0x4a,sec);//秒变化时重新写入
							write_com(0x80+0x4b);//写入后将光标移回并闪烁，显示调整状态
						}
					if (addr==2) 
						{
							min--;
							if (min==-1)
									min=59;
							display(0x80+0x47,min);//分变化时重新写入
							write_com(0x80+0x48);//写入后将光标移回并闪烁，显示调整状态
						}
					if (addr==3) 
						{
							hour--;
							if (hour==-1)
								hour=23;
							display(0x80+0x44,hour);//时变化时重新写入
							write_com(0x80+0x45);//写入后将光标移回并闪烁，显示调整状态
						}
				}
		}
}

/*蜂鸣器函数*/
void beeper()
{
	beep=0;				//开启蜂鸣器
	delayms(100);		//0.1s延时
	beep=1;				//关闭蜂鸣器
}

/*液晶显示函数*/
void display(uchar add,uchar date)//显示时分秒函数。add什么位置 date显示什么
{
	uchar shi,ge;
	shi=date/10;	
	ge=date%10;

	write_com(add);
	write_data(shuzi[shi]);
	write_data(shuzi[ge]);
}


