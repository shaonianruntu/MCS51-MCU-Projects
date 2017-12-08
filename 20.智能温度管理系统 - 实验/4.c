#include<reg52.h>
#include<intrins.h>
#define uint unsigned int
#define uchar unsigned  char
#define MAIN_Fosc	11059200UL	//宏定义主时钟HZ
#define AT24C02_ADD 0xa0            //AT24C02地址
/*数码管,需关闭*/
sbit dula=P2^6;	  //数码管段选
sbit wela=P2^7;	  //数码管位选
/*液晶*/
sbit lcdrs=P3^5;		   //RS液晶数据命令选择端
sbit lcdrw=P3^6;		   //WR液晶读写选择端
sbit lcden=P3^4;		   //LCDEN液晶使能端
/*独立按键*/
sbit s1=P3^0;   //阈值模式设置界面切换键
sbit s2=P3^1;	//阈值选择切换键
sbit s3=P3^2;	//阈值增大键  \ 保存翻墙平均温度
sbit s4=P3^3;	//阈值减小键  \ 调出保存数据
/*蜂鸣器*/
sbit beep=P2^3; //定义蜂鸣器端
/*温度传感器*/
sbit DS=P2^2;	  //DS18B20单总线(信号线)
/*E2PROM*/
sbit SDA=P2^0;						//时钟总线
sbit SCL=P2^1;						//数据总线


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
/*温度传感器*/
bit DS_init();	  //单总线初始化函数
void DS_set();			//温度阈值设置界面
void DS_wendu();			//DS18B20获取温度值
uchar DS_read_byte();	//DS18B20读数据
void DS_write_byte(uchar date);	//DS18B20写数据
void DS_display(uchar,uint);//温度液晶显示函数
/*E2PROM*/
void i2c_init();	 //i2c初始化
void i2c_start();	 //启动i2c
void i2c_stop();	 //停止i2c
void i2c_sendbyte(uchar bt);	//将输入的一字节数据bt发送
uchar i2c_recbyte(); //从总线上接收1字节数据
void i2c_respons();  //i2c应答信号  
void i2c_write(uchar address, char dat);//在指定地址写入一字节数据
char i2c_read(uchar address);//从指定地址读出一字节数据
/*液晶*/
void yj_display(uchar add,uchar date);//液晶显示函数
void yj_weekdisplay(uchar a);//星期显示函数
void yj_write_com (uchar com);//液晶命令写入
void yj_write_data(uchar date);//液晶数据写入
/*延时*/
void delayms(uint xms);		//ms级显示函数
void delayus(uchar xus);	//us级显示函数
void delay();			//4us延时

void keyscan();		//键盘扫描函数
void T_init();		//定时器初始化
void init();//初始化函数
void beeper();//蜂鸣器函数

uchar temp;	//temp:实时温度,
uchar maximun,minimun;//maximun:温度上阈值,minimun:温度下阈值		
char tou,nian,yue,ri,week,hour,min,sec;
char setting,s2num,count;//count:定时器计数,setting:阈值设置模式进入标志,s2num:按键次数

/**************************************************
**************************************************/
void main()
{
	init();//首先初始化各数据
	while(1)
	{
		keyscan();
		DS_wendu();
	}
}

/*初始化函数*/
void init()
{
	dula=0;
	wela=0;	 //关闭段选和位选,防止出现数码管显示
	lcden=0; //关闭液晶使能端
	
	/*初始化各种变量值*/
	tou=20;
	nian=16;
	yue=1;
	ri=31;
	week=1;
	hour=12;
	min=0;
	sec=0;	  //时钟,模式下的变量
	maximun=0;
	minimun=0;//温度阈值初始化
	/*初始化i2c*/
	i2c_init();	
	/*初始化1602液晶*/
	yj_write_com(0x38);		 //00111000B,设置为16*2显示，5*7点阵，8位数据接口
	yj_write_com(0x0c);		 //00001100B,设置开显示，不显示光标
	yj_write_com(0x06);		 //00000110B,写一个字符后地址指针加1
	yj_write_com(0x01);		 //00000001B,显示清0，数据指针清0
	/*液晶显示*/
	yj_display(0x80,tou);		 //(1,1)显示年份的开头20
	yj_display(0x80+2,nian);	 //(1,3)显示年份的后两位
	yj_write_data('-');
	yj_display(0x80+5,yue);	 //(1,6)显示月份
	yj_write_data('-');
	yj_display(0x80+8,ri);		 //(1,9)显示日期
	yj_weekdisplay(week);		 //(1,14)显示星期
	yj_display(0x80+0x40,hour); //(2,5)显示小时
	yj_write_data(':');
	yj_display(0x80+0x40+3,min);	 //(2,8)显示分钟
	yj_write_data(':');
	yj_display(0x80+0x40+6,sec);	 //(2,11)显示秒

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
/*键盘扫描函数*/
void keyscan()
{
	if(0==s1)
	{
		delayms(5);
		if(0==s1)//确认阈值设置键已经按下
		{
			while(!s1);//确认释放
			beeper();//每当有按键释放蜂鸣器发出滴声
			P1=0;	//开启设置模式,亮灯作为反馈
			delayms(200);  //延时0.2s
			P1=0xff;
			TR0=0;	//关闭定时器中断
			setting=1;	//进入温度阈值设置模式
			DS_set(); //进入温度阈值设置界面
			
			do
			{
				if(0==s2)
				{
					delayms(5);
					if(0==s2)//确认功能键被按下
					{
						s2num++;//功能键按下次数记录
						while(!s2);//释放确认
						beeper();//每当有按键释放蜂鸣器发出滴声	   
						if(s2num==1)//第一次被按下时
						{
							yj_write_com(0x80+9);//光标定位到最大温度阈值位置(1,10)	
							yj_write_com(0x0f); //光标开始闪烁
						}
						if(s2num==2)//第二次按下光标闪烁定位到最小阈值位置
							yj_write_com(0x80+0x40+9);//(2,10)
						if(s2num==3)//第三次按下
						{
							s2num=0;//记录按键数清零
							yj_write_com(0x0c);//取消光标闪烁
						}
					}
				}
				if(s2num!=0)//只有功能键被按下后，增加和减小键才有效
				{
					if(0==s3)
					{
						delayms(5);
						if(0==s3)//增加键确认被按下
						{
							while(!s3);//按键释放
							beeper();//每当有按键释放蜂鸣器发出滴声
							if(s2num==1)//若功能键第一次按下
							{
								maximun++;
								yj_display(0x80+8,maximun);//每调节一次送液晶显示一下			
								yj_write_com(0x80+9);//显示位置重新回到调节处
							}
							if(s2num==2)//若功能键第二次按下
							{
								minimun++;
								yj_display(0x80+0x40+8,minimun);//每调节一次送液晶显示一下
								yj_write_com(0x80+0x40+9);//显示位置重新回到调节处
							}
						}
					}
					if(0==s4)
					{
						delayms(5);
						if(0==s4)//确认减小键被按下
						{
							while(!s4);//按键释放
							beeper();//每当有按键释放蜂鸣器发出滴声
							if(s2num==1)//若功能键第一次按下
							{
								maximun--;
								if(maximun<0)	 //不允许负数
									maximun=0;
								yj_display(0x80+8,maximun);//每调节一次送液晶显示一下
								yj_write_com(0x80+9);//显示位置重新回到调节处
							}
							if(s2num==2)
							{
								minimun--;
								if(minimun<0);
									minimun=0;
								yj_display(0x80+0x40+8,minimun);//每调节一次送液晶显示一下
								yj_write_com(0x80+9);//显示位置重新回到调节处
							}
						}
					}
				}
				if(0==s1)
				{
					delayms(5);
					if(0==s1)//确认阈值设置键已经按下
					{
						while(!s1);//按键释放
						beeper();//每当有按键释放蜂鸣器发出滴声
						P1=0;	//开启设置模式,亮灯作为反馈
						delayms(200);  //延时0.2s
						P1=0xff;
						TR0=1;	//关闭定时器中断
						setting=0;

						yj_write_com(0x01);		 //00000001B,显示清0，数据指针清0

						yj_display(0x80,tou);
						yj_display(0x80+2,nian);
						yj_write_data('-');
						yj_display(0x80+5,yue);
   						yj_write_data('-');
						yj_display(0x80+8,ri);
						yj_weekdisplay(week);
						yj_display(0x80+0x40,hour);
						yj_write_data(':');
						yj_display(0x80+0x40+3,min);
						yj_write_data(':');
						yj_display(0x80+0x40+6,sec);
						DS_wendu();
					}
				}
			}while(setting);	
		}
	}
	
	if(s1!=0)
	{
		if(0==s3)
		{
			delayms(5);
			if(0==s3)
			{
				while(!s3);//按键释放
				beeper();//每当有按键释放蜂鸣器发出滴声
				i2c_write(10,temp);//保存当前时刻的平均温度
			}
		}
		if(0==s4)
		{
			delayms(5);
			if(0==s4)
			{
				while(!s4);//按键释放
				beeper();//每当有按键释放蜂鸣器发出滴声
			
				nian=i2c_read(1);
				yue=i2c_read(2);
				ri=i2c_read(3);
				week=i2c_read(4);
				hour=i2c_read(5);
				min=i2c_read(6);
				sec=i2c_read(7);
				temp=i2c_read(10);
				maximun=i2c_read(11);
				minimun=i2c_read(12);

				yj_display(0x80,tou);		 //(1,1)显示年份的开头20
				yj_display(0x80+2,nian);	 //(1,3)显示年份的后两位
				yj_write_data('-');
				yj_display(0x80+5,yue);	 //(1,6)显示月份
				yj_write_data('-');
				yj_display(0x80+8,ri);		 //(1,9)显示日期
				yj_weekdisplay(week);		 //(1,14)显示星期
				yj_display(0x80+0x40,hour); //(2,1)显示小时
				yj_write_data(':');
				yj_display(0x80+0x40+3,min);	 //(2,4)显示分钟
				yj_write_data(':');
				yj_display(0x80+0x40+6,sec);	 //(2,7)显示秒
				yj_display(0x80+0x40+12,temp);	  //(2,13)显示温度
				yj_write_data(0xdf);
				yj_write_data('C');
			}
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


/*******************************************************
*********************DS18B20函数************************
*******************************************************/
/*温度阈值设置界面*/
void DS_set()
{
	yj_write_com(0x01);	  //00000001B,显示清0，数据指针清0
	
	yj_write_com(0x80+4);
	yj_write_data('M');
	yj_write_data('A');
	yj_write_data('X');
	yj_display(0x80+8,maximun);
	yj_write_data(0xdf);
	yj_write_data('C');

	yj_write_com(0x80+0x40+4);
	yj_write_data('M');
	yj_write_data('I');
	yj_write_data('N');
	yj_display(0x80+0x40+8,minimun);
	yj_write_data(0xdf);
	yj_write_data('C');
}

/*DS18B20获取温度值*/
void DS_wendu()
{
	uchar i;
	uchar L,M;	 //L:温度值低位,M:温度值高位
	
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
	temp = i* 0.0625+0.5;//转换为十进制,并四舍五入小数
	yj_display(0x80+0x40+12,temp);
	yj_write_data(0xdf);
	yj_write_data('C');
}

/*液晶显示函数
void DS_display(uchar add,uint date)//显示时分秒函数。add什么位置 date显示什么
{
	uint shi,ge;
	shi=date/10;	
	ge=date%10;

	yj_write_com(add);
	yj_write_data(shi+'0');
	yj_write_data(ge+'0');
} */

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
	TMOD=0x01;//设置定时器0工作模式1
	TH0=(65536-45872)/256;//定时器装初值
	TL0=(65536-45872)%256;

	ET0=1;						  //开定时器0中断
	TR0=1;						  //启动定时器0
	EA=1;						  //开总中断
}

/*定时器0服务设置*/
void T0_time0() interrupt 1//定时器0中断服务程序
{
	TH0=(65536-45872)/256;//再次装定时器初值
	TL0=(65536-45872)%256;
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
				yj_display(0x80+0x40,hour);//小时若变化则重新写入
				i2c_write(5,hour);//数据改变立即存入24C02
			}
			yj_display(0x80+0x40+3,min);//分钟若变化则重新写入
			i2c_write(6,min);//数据改变立即存入24C02
		}
		yj_display(0x80+0x40+6,sec);//秒若变化则重新写入	
		i2c_write(7,sec);//数据改变立即存入24C02
	}	
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

/*4us延时*/
void delay()
{
;;
}