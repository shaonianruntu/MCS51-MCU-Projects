/********************************************************************************
程序：用定时器编写电子时钟程序，设定三个功能键，一个功能键s1，一个数值增大键s2；一个数
值减小键s3来调整时间。并设置s4作为闹钟模式和时钟模式切换键,同时可以使用三个功能键对闹
钟时间进行调整.同时具有串口通信功能,能够通过串口通信对时间进行修改(我定义的修改模式为
输入n后等待光标闪烁,在对nian进行修改;同理输入y,r,w,h,m,s等待1602液晶屏上对应位置的光标
闪烁后再对其进行修改).
并且本程序能将数据存入到AT2404中，再次启动电源，电子表从断电时刻开始继续计时
**********************************************************************************/

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
sbit s4=P3^3;	//模式切换键

sbit beep=P2^3; //定义蜂鸣器端

void delayms(uint xms);	//ms级延时
void delay();			//4us延时

void i2c_init();	 //i2c初始化
void i2c_start();	 //启动i2c
void i2c_stop();	 //停止i2c
void i2c_sendbyte(uchar bt);	//将输入的一字节数据bt发送
uchar i2c_recbyte(); //从总线上接收1字节数据
void i2c_respons();  //i2c应答信号  
void i2c_write(uchar address, char dat);//在指定地址写入一字节数据
char i2c_read(uchar address);//从指定地址读出一字节数据

void yj_display(uchar add,uchar date);//液晶显示函数
void yj_weekdisplay(uchar a);//星期显示函数
void yj_write_com (uchar com);//液晶命令写入
void yj_write_data(uchar date);//液晶数据写入

void nz_clock();	 //闹钟模式显示函数
void nz_ONorOFF(uchar b);//闹钟开关设置
void nz_time();		//判断是否到达闹钟设置的时间

void ck_input();	//获取串口数据
	
void keyscan();		//键盘扫描函数
void T_init();		//定时器初始化
void init();//初始化函数
void beeper();//蜂鸣器函数


uchar table[14]="ALARM MODE SET";

uchar count,s1num; //count:记录中断次数
				   //s1num:记录s1按键按下的次数
uchar alarm,setting;//alarm:记录是否设置过闹钟
					//setting:标志是否设置完成
uchar chuankou,flag;//chuankou:储存串口通信接收的数据
					//flag:标记是否接收到串口数据
char tou,nian,yue,ri,week,hour,min,sec;//设置年月日时分秒位
char ialarm,ihour=12,imin=0,isec=0;//设置闹钟模式时分秒,ialarm为闹钟开关


/*************************************************
**************************************************
*************************************************/
/*主函数*/
void main()
{
	init();//首先初始化各数据
	flag=0;	//串口通信接收标志位清零,
	while(1)//进入主程序大循环
	{
		if(flag==1)
		{
			ck_input();
		}

		keyscan();//不停的检测按键是否被按下
		nz_time(); //判断是否进入闹钟响铃
	}	
}

/*初始化函数*/
void init()
{
	dula=0;
	wela=0;			//关闭两锁存器锁存端，防止操作液晶时数码管会出乱码
	lcden=0;		//关闭液晶使能端
	
	/*初始化各种变量值*/
	tou=20;
	nian=16;
	yue=1;
	ri=31;
	week=1;
	hour=12;
	min=0;
	sec=0;	  //时钟,模式下的变量
	ialarm=0;
	ihour=12;
	imin=0;	  //闹钟模式下的变量
	count=0;  //中断次数清零
	s1num=0;  //s1按键次数清零
	/*初始化i2c*/
	i2c_init();	
	/*初始化1602液晶*/
	yj_write_com(0x38);		 //00111000B,设置为16*2显示，5*7点阵，8位数据接口
	yj_write_com(0x0c);		 //00001100B,设置开显示，不显示光标
	yj_write_com(0x06);		 //00000110B,写一个字符后地址指针加1
	yj_write_com(0x01);		 //00000001B,显示清0，数据指针清0

	nian=i2c_read(1);
	yue=i2c_read(2);
	ri=i2c_read(3);
	week=i2c_read(4);
	hour=i2c_read(5);
	min=i2c_read(6);
	sec=i2c_read(7);
	ialarm=i2c_read(11);
	ihour=i2c_read(12);
	imin=i2c_read(13);

	yj_display(0x80,tou);		 //(1,1)显示年份的开头20
	yj_display(0x80+2,nian);	 //(1,3)显示年份的后两位
	yj_write_data('-');
	yj_display(0x80+5,yue);	 //(1,6)显示月份
	yj_write_data('-');
	yj_display(0x80+8,ri);		 //(1,9)显示日期
	yj_weekdisplay(week);		 //(1,14)显示星期
	yj_display(0x80+0x44,hour); //(2,5)显示小时
	yj_write_data(':');
	yj_display(0x80+0x47,min);	 //(2,8)显示分钟
	yj_write_data(':');
	yj_display(0x80+0x4a,sec);	 //(2,11)显示秒

	nian=i2c_read(1);
	yue=i2c_read(2);
	ri=i2c_read(3);
	week=i2c_read(4);
	hour=i2c_read(5);
	min=i2c_read(6);
	sec=i2c_read(7);
	/*初始化定时器*/
	T_init();	
}

/*蜂鸣器函数*/
void beeper()
{
	beep=0;
	delayms(100);
	beep=1;
}


/*************************************************
********************键盘扫描**********************
**************************************************/
/*按键扫描函数*/
void keyscan()
{
	if(s4!=0)
	{
		if(s1==0)	
		{
			delayms(5);
			if(s1==0)//确认功能键被按下
			{	
				s1num++;//功能键按下次数记录
				while(!s1);//释放确认
				beeper();//每当有按键释放蜂鸣器发出滴声
				if(s1num==1)//第一次被按下时
				{
					TR0=0;  //关闭定时器
					yj_write_com(0x80+0x40+11);//秒位(2,12)
					yj_write_com(0x0f); //光标开始闪烁
				}
				if(s1num==2)//第二次按下光标闪烁定位到分位
					yj_write_com(0x80+0x40+8);//(2,8)
				if(s1num==3)//第三次按下光标闪烁定位到小时位
					yj_write_com(0x80+0x40+5);//(2,6)
				if(s1num==4)//第四次按下光标闪烁定位到星期位
					yj_write_com(0x80+15);//(1,16)
				if(s1num==5)//第五次按下光标闪烁定位到日位
					yj_write_com(0x80+9);//(1,10)
				if(s1num==6)//第六次按下光标闪烁定位到月位
					yj_write_com(0x80+6);//(1,7)
				if(s1num==7)//第七次按下光标闪烁定位到年位
					yj_write_com(0x80+3);//(1,4)
				if(s1num==8)//第八次按下
				{
					s1num=0;//记录按键数清零
					yj_write_com(0x0c);//取消光标闪烁
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
						sec++; //则调整秒加1
						if(sec==60)//若满60后将清零
							sec=0;
						yj_display(0x80+0x40+10,sec);//每调节一次送液晶显示一下
						yj_write_com(0x80+0x40+11);//显示位置重新回到调节处
						i2c_write(7,sec);	//数据改变立即存入24C02
					}
					if(s1num==2)//若功能键第二次按下
					{
						min++;//则调整分钟加1
						if(min==60)//若满60后将清零
							min=0;
						yj_display(0x80+0x40+7,min);//每调节一次送液晶显示一下
						yj_write_com(0x80+0x40+8);//显示位置重新回到调节处
						i2c_write(6,min);//数据改变立即存入24C02
					}
					if(s1num==3)//若功能键第三次按下
					{
						hour++;//则调整小时加1
						if(hour==24)//若满24后将清零
							hour=0;
						yj_display(0x80+0x40+4,hour);//每调节一次送液晶显示一下
						yj_write_com(0x80+0x40+5);//显示位置重新回到调节处
						i2c_write(5,hour);//数据改变立即存入24C02
					}
					if(s1num==4)//若功能键第四次按下
					{
						week++;//则调整小时加1
						if(week==8)//若满24后将清零
							week=1;
						yj_weekdisplay(week);
						yj_write_com(0x80+15);//显示位置重新回到调节处
						i2c_write(4,week);//数据改变立即存入24C02
					}
					if(s1num==5)//若功能键第五次按下
					{
						ri++;//则调整小时加1
						if(ri==32)//若满24后将清零
							ri=1;
						yj_display(0x80+8,ri);//每调节一次送液晶显示一下
						yj_write_com(0x80+9);//显示位置重新回到调节处
						i2c_write(3,ri);//数据改变立即存入24C02
					}
					if(s1num==6)//若功能键第六次按下
					{
						yue++;//则调整小时加1
						if(yue==32)//若满24后将清零
							yue=1;
						yj_display(0x80+5,yue);//每调节一次送液晶显示一下
						yj_write_com(0x80+6);//显示位置重新回到调节处
						i2c_write(2,yue);//数据改变立即存入24C02
					}	
					if(s1num==7)//若功能键第七次按下
					{
						nian++;//则调整小时加1
						if(nian==100)//若满24后将清零
							nian=0;
						yj_display(0x80+2,nian);//每调节一次送液晶显示一下
						yj_write_com(0x80+3);//显示位置重新回到调节处
						i2c_write(1,nian);//数据改变立即存入24C02
					}	
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
					sec--;//则调整秒减1
					if(sec==-1)//若减到负数则将其重新设置为59
						sec=59;
					yj_display(0x80+0x40+10,sec);//每调节一次送液晶显示一下
					yj_write_com(0x80+0x40+11);//显示位置重新回到调节处
					i2c_write(7,sec);//数据改变立即存入24C02
				}
				if(s1num==2)//若功能键第二次按下
				{
					min--;//则调整分钟减1
					if(min==-1)//若减到负数则将其重新设置为59
						min=59;
					yj_display(0x80+0x40+7,min);//每调节一次送液晶显示一下
					yj_write_com(0x80+0x40+8);//显示位置重新回到调节处
					i2c_write(6,min);//数据改变立即存入24C02
				}
				if(s1num==3)//若功能键第二次按下
				{
					hour--;//则调整小时减1
					if(hour==-1)//若减到负数则将其重新设置为23
						hour=23;
					yj_display(0x80+0x40+4,hour);//每调节一次送液晶显示一下
					yj_write_com(0x80+0x40+5);//显示位置重新回到调节处
					i2c_write(5,hour);//数据改变立即存入24C02
				}
				if(s1num==4)//若功能键第二次按下
				{
					week--;//则调整小时减1
					if(week==0)//若减到负数则将其重新设置为23
						week=7;
					yj_weekdisplay(week);
					yj_write_com(0x80+15);//显示位置重新回到调节处
					i2c_write(4,week);//数据改变立即存入24C02
				}
				if(s1num==5)//若功能键第二次按下
				{
					ri--;//则调整小时减1
					if(ri==0)//若减到负数则将其重新设置为23
						ri=31;
					yj_display(0x80+8,ri);//每调节一次送液晶显示一下
					yj_write_com(0x80+9);//显示位置重新回到调节处
					i2c_write(3,ri);//数据改变立即存入24C02
				}
				if(s1num==6)//若功能键第二次按下
				{
					yue--;//则调整小时减1
					if(yue==0)//若减到负数则将其重新设置为23
						yue=12;
					yj_display(0x80+5,yue);//每调节一次送液晶显示一下
					yj_write_com(0x80+6);//显示位置重新回到调节处
					i2c_write(2,yue);//数据改变立即存入24C02
				}
				if(s1num==7)//若功能键第二次按下
				{
					nian--;//则调整小时减1
					if(nian==-1)//若减到负数则将其重新设置为23
						nian=99;
					yj_display(0x80+2,nian);//每调节一次送液晶显示一下
					yj_write_com(0x80+3);//显示位置重新回到调节处
					i2c_write(1,nian);//数据改变立即存入24C02
				}
			}
		}
	}

	if(0==s4)
	{
		delayms(5);
		if(0==s4)//确认功能键被按下
		{
			while(!s4);//释放确认
			beeper();//每当有按键释放蜂鸣器发出滴声	
			P1=0;	 //开启闹铃模式,亮灯反馈
			delayms(200); //延时0.2s
			P1=0xff;
			TR0=0;		//关闭定时器中断
			alarm=1;	 //开闹钟
			setting=1;	 //进入闹钟设置状态
			nz_clock();	//进入闹钟界面
			
			do
			{
				if (s1==0)
				{
					delayms(5);		   
					if (s1==0)//确认功能键被按下
					{
						s1num++;//功能键按下次数记录					
						while(!s1);//释放确认
						beeper();//每当有按键释放蜂鸣器发出滴声	   
						if(s1num==1)//第一次被按下时
						{
							TR0=0;  //关闭定时器
							yj_write_com(0x80+0x40+2);//光标定位到闹钟开关选择位位置(2,3)
							yj_write_com(0x0f); //光标开始闪烁
						}
						if(s1num==2)//第二次按下光标闪烁定位到分钟位置
							yj_write_com(0x80+0x40+7);//(2,8)
						if(s1num==3)//第三次按下光标闪烁定位到小时位置
							yj_write_com(0x80+0x40+10);//(2,11)
						if(s1num==4)//第四次按下
						{
							s1num=0;//记录按键数清零
							yj_write_com(0x0c);//取消光标闪烁
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
								ialarm++; //则调整秒加1
								if(ialarm==2)//若满60后将清零
									ialarm=0;
								nz_ONorOFF(ialarm);
								yj_write_com(0x80+0x40+2);//显示位置重新回到调节处
								i2c_write(11,ialarm);	//数据改变立即存入24C02
							}
							if(s1num==2)//若功能键第二次按下
							{
								ihour++;//则调整分钟加1
								if(ihour==24)//若满60后将清零
									ihour=0;
								yj_display(0x80+0x40+6,ihour);//每调节一次送液晶显示一下
								yj_write_com(0x80+0x40+7);//显示位置重新回到调节处
								i2c_write(12,ihour);//数据改变立即存入24C02
							}
							if(s1num==3)//若功能键第三次按下
							{
								imin++;//则调整小时加1
								if(imin==60)//若满24后将清零
									imin=0;
								yj_display(0x80+0x40+9,imin);//每调节一次送液晶显示一下
								yj_write_com(0x80+0x40+10);//显示位置重新回到调节处
								i2c_write(13,imin);//数据改变立即存入24C02
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
								ialarm--;//则调整秒减1
								if(ialarm==-1)//若减到负数则将其重新设置为59
									ialarm=1;
								 nz_ONorOFF(ialarm);
							   	 yj_write_com(0x80+0x40+2);//显示位置重新回到调节处
							  	 i2c_write(11,ialarm);//数据改变立即存入24C02
							}
							if(s1num==2)//若功能键第二次按下
							{
								ihour--;//则调整分钟减1
								if(ihour==-1)//若减到负数则将其重新设置为59
									ihour=23;
								yj_display(0x80+0x40+6,ihour);//每调节一次送液晶显示一下
								yj_write_com(0x80+0x40+7);//显示位置重新回到调节处
								i2c_write(12,ihour);//数据改变立即存入24C02
							}
							if(s1num==3)//若功能键第二次按下
							{
								imin--;//则调整小时减1
								if(imin==-1)//若减到负数则将其重新设置为23
									imin=59;
								yj_display(0x80+0x40+9,imin);//每调节一次送液晶显示一下
								yj_write_com(0x80+0x40+10);//显示位置重新回到调节处
								i2c_write(13,imin);//数据改变立即存入24C02
							}
						}
					}
				}
				if(s4==0)
				{
					delayms(5);		   //延时防抖
					if(s4==0)			   //重复判断,防抖
					{
						while(!s4);
						beeper();		   //按键按下时蜂鸣器响,作为反馈
						P1=0;			   //开启闹钟模式,亮灯反馈
						delayms(200);	   //延时
						P1=0xff;
						TR0=1;
						setting=0;		   //设置完成

						yj_write_com(0x01);		 //00000001B,显示清0，数据指针清0

						yj_display(0x80,tou);
						yj_display(0x80+2,nian);
						yj_write_data('-');
						yj_display(0x80+5,yue);
   						yj_write_data('-');
						yj_display(0x80+8,ri);
						yj_weekdisplay(week);
						yj_display(0x80+0x44,hour);
						yj_write_data(':');
						yj_display(0x80+0x47,min);
						yj_write_data(':');
						yj_display(0x80+0x4a,sec);
					}
				}
			}while(setting); //等待设置完成
		}
	}
}


/*************************************************
********************闹钟设置**********************
**************************************************/
/*闹钟模式显示*/
void nz_clock()
{
	uchar i;

	yj_write_com(0x01);		 //00000001B,显示清0，数据指针清0
	yj_write_com (0x80+0x01);
	for (i=0;i<14;i++)
	{
		yj_write_data(table[i]);	
	}
	yj_write_com(0x80+0x40+1);
	nz_ONorOFF(ialarm);
	yj_display(0x80+0x40+6,ihour);
	yj_write_data(':');
	yj_display(0x80+0x40+9,imin);		
}

/*闹钟模式选择函数*/
void nz_ONorOFF(uchar b)
{
	if(0==b)
	{
		yj_write_com(0x80+0x40+1);
		yj_write_data('O');
		yj_write_data('N');
		yj_write_data(' ');
	}
	if(1==b)
	{
		yj_write_com(0x80+0x40+1);
		yj_write_data('O');
		yj_write_data('F');
		yj_write_data('F');
	}
}

/*闹钟时间判断*/
void nz_time()
{
	uint i;
	if(0==ialarm&&1==alarm)	//设置过闹钟且允许闹钟响
	{
		if((hour==ihour)&&(min==imin))
		{
			beep=0;				 
			for(i=0;i<60;i++)
				delayms(1000);
			beep=1;				//蜂鸣器响1s
			alarm=0;
		}
	}
}

/*************************************************
******************1602液晶显示********************
**************************************************/
/*液晶命令写入*/
void yj_write_com (uchar com)
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
void yj_write_data(uchar date)
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
void yj_display(uchar add,uchar date)//显示时分秒函数。add什么位置 date显示什么
{
	uchar shi,ge;
	shi=date/10;	
	ge=date%10;

	yj_write_com(add);
	yj_write_data(shi+'0');
	yj_write_data(ge+'0');
}

/*星期显示*/
void yj_weekdisplay(uchar a)
{
	switch(a)
	{
		case 1:	yj_write_com(0x80+13);
				yj_write_data('M');
				yj_write_data('0');
				yj_write_data('N');
				break;
		case 2:	yj_write_com(0x80+13);
				yj_write_data('T');
				yj_write_data('U');
				yj_write_data('E');
				break;
		case 3:	yj_write_com(0x80+13);
				yj_write_data('W');
				yj_write_data('E');
				yj_write_data('D');
				break;
		case 4:	yj_write_com(0x80+13);
				yj_write_data('T');
				yj_write_data('H');
				yj_write_data('U');
				break;
		case 5:	yj_write_com(0x80+13);
				yj_write_data('F');
				yj_write_data('R');
				yj_write_data('I');
				break;
		case 6:	yj_write_com(0x80+13);
				yj_write_data('S');
				yj_write_data('A');
				yj_write_data('T');
				break;
		case 7:	yj_write_com(0x80+13);
				yj_write_data('S');
				yj_write_data('U');
				yj_write_data('N');
				break;
	}
}


/*************************************************
******************i2c断电保存*********************
**************************************************/
/*i2c初始化*/
void i2c_init()			//将总线都拉高,以释放总线	   
{
	SDA=1;
	delay();
	SCL=1;					   
	delay();
}

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

 
/*************************************************
*******************定时器设置*********************
**************************************************/
/*定时器初始化*/
void T_init()
{
	TMOD=0x21;//设置定时器1工作方式2,定时器0工作模式1
	TH0=(65536-50000)/256;//定时器装初值
	TL0=(65536-50000)%256;
	TH1=253;					  //T1定时器赋初值,比特率9600
	TL1=253;	

	ET0=1;						  //开定时器0中断
	TR0=1;						  //启动定时器0
	TR1=1;						  //启动定时器1

	REN=1;						  //允许串口接受数据
	SM0=0;						  //设定串口工作方式1
	SM1=1;

	EA=1;						  //开总中断
	ES=1;						  //开串口中断	
}

/*定时器0服务设置*/
void T0_time0() interrupt 1//定时器0中断服务程序
{
	TH0=(65536-50000)/256;//再次装定时器初值
	TL0=(65536-50000)%256;
	count++;		//中断次数累加
	if(count==20) 	//20次50毫秒为1秒
	{
		count=0;
		sec++;
		if(sec==60)//秒加到60则进位分钟
		{
			sec=0;//同时秒数清零
			min++;
			if(min==60)//分钟加到60则进位小时
			{
				min=0;//同时分钟数清零
				hour++;
				if(hour==24)//小时加到24则小时清零
				{
					hour=0;
					week++;
					if(week==8)
						week=1;
					ri++;
					if(ri==32)
					{
						ri=1;
						yue++;
						if(yue==13)
						{
							yue=1;
							nian++;
							yj_display(0x80+2,nian);//年变化则重新写入年
							i2c_write(1,nian);//数据改变立即存入24C02
						}
						yj_display(0x80+5,yue);//月变化则重新写月
						i2c_write(2,yue);//数据改变立即存入24C02
					}
					yj_display(0x80+8,ri);//日变化则重新写入日
					yj_weekdisplay(week);//周变化则重新写入周
					i2c_write(3,ri);//数据改变立即存入24C02
					i2c_write(4,week);//数据改变立即存入24C02
				}
				yj_display(0x80+0x40+4,hour);//小时若变化则重新写入
				i2c_write(5,hour);//数据改变立即存入24C02
			}
			yj_display(0x80+0x40+7,min);//分钟若变化则重新写入
			i2c_write(6,min);//数据改变立即存入24C02
		}
		yj_display(0x80+0x40+10,sec);//秒若变化则重新写入	
		i2c_write(7,sec);//数据改变立即存入24C02
	}	
}


/*************************************************
******************串口通信设置********************
**************************************************/
/*串口中断服务*/
void ck_ser() interrupt 4
{
	RI=0;					//软件清楚接收中断
	chuankou=SBUF;		//把数据存入到数据数组
	flag=1;					//标志是否已收到数据
} 

/*获取串口数据*/
void ck_input()
{
	if(chuankou=='n')
	{
		TR0=0;				  //关闭时钟走时
		yj_write_com(0x80+3);	 //光标移到年位置(第一排4位)
		yj_write_com(0x0f);	 //光标闪烁
		
		while(!RI);
			RI=0;
		nian=SBUF;	
		i2c_write(1,nian);//数据改变立即存入24C02
		yj_display(0x80+2,nian);//年变化时重新写入	
		yj_write_com(0x0c);		//开显示,但取消光标闪烁
		flag=0;					
		TR0=1;					//打开时钟走时
	}
	if(chuankou=='y')
	{
		TR0=0;				  //关闭时钟走时
		yj_write_com(0x80+6);	 //光标移到月位置(第一排7位)
		yj_write_com(0x0f);		 //光标闪烁
	
		while(!RI);
		RI=0;
		yue=SBUF;
		i2c_write(2,yue);//数据改变立即存入24C02
		yj_display(0x80+5,yue);//月变化时重新写入	
		yj_write_com(0x0c);		  //开显示,但取消光标闪烁
		flag=0;
		TR0=1;					//打开时钟走时
	}
	if(chuankou=='r')
	{
		TR0=0;				  //关闭时钟走时
		yj_write_com(0x80+9);	 //光标移到日位置(第一排10位)
		yj_write_com(0x0f);		 //光标闪烁
				
		while(!RI);
			RI=0;
		ri=SBUF;
		i2c_write(3,ri);//数据改变立即存入24C02
		yj_display(0x80+8,ri);//日变化时重新写入	
		yj_write_com(0x0c);		  //开显示,但取消光标闪烁
		flag=0;
		TR0=1;					//打开时钟走时
	}
	if(chuankou=='w')
	{
		TR0=0;				  //关闭时钟走时
		yj_write_com(0x80+15);	 //光标移到星期位置(第一排16位)
		yj_write_com(0x0f);		 //光标闪烁
			
		while(!RI);
			RI=0;
		week=SBUF;
		i2c_write(4,week);//数据改变立即存入24C02
		yj_weekdisplay(week);	
		yj_write_com(0x0c);		  //开显示,但取消光标闪烁
		flag=0;
		TR0=1;					//打开时钟走时
	}
	if(chuankou=='h')
	{
		TR0=0;				  //关闭时钟走时
		yj_write_com(0x80+0x45);	 //光标移到时位置(第一排16位)
		yj_write_com(0x0f);		 //光标闪烁
			
		while(!RI);
			RI=0;
		hour=SBUF;
		i2c_write(5,hour);//数据改变立即存入24C02
		yj_display(0x80+0x44,hour);//星期变化时重新写入	
		yj_write_com(0x0c);		  //开显示,但取消光标闪烁
		flag=0;
		TR0=1;					//打开时钟走时
	}
	if(chuankou=='m')
	{
		TR0=0;				  //关闭时钟走时
		yj_write_com(0x80+0x48);	 //光标移到星期位置(第一排16位)
		yj_write_com(0x0f);		 //光标闪烁
			
		while(!RI);
			RI=0;
		min=SBUF;
		i2c_write(6,min);//数据改变立即存入24C02
		yj_display(0x80+0x47,min);//星期变化时重新写入	
		yj_write_com(0x0c);		  //开显示,但取消光标闪烁
		flag=0;
		TR0=1;					//打开时钟走时
	}		   
	if(chuankou=='s')
	{
		TR0=0;				  //关闭时钟走时
		yj_write_com(0x80+0x4b);	 //光标移到星期位置(第一排16位)
		yj_write_com(0x0f);		 //光标闪烁		
						
		while(!RI);
			RI=0;
		sec=SBUF;
		i2c_write(7,sec);//数据改变立即存入24C02
		yj_display(0x80+0x4a,sec);//星期变化时重新写入	
		yj_write_com(0x0c);		  //开显示,但取消光标闪烁
		flag=0;
		TR0=1;					//打开时钟走时
	}	 	  	
}

/*************************************************
******************延时函数设置********************
**************************************************/
/*ms级延时函数*/
void delayms(uint xms)
{
	uint i,j;
	for(i=xms;i>0;i--)
		for(j=114;j>0;j--);
}

/*4us延时*/
void delay()
{
;;
}



