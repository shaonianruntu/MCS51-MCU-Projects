/********************************************************************************
程序：用定时器编写电子时钟程序，设定三个功能键，一个功能键s1，一个数值增大键s2；一个数
值减小键s3来调整时间。并设置s4作为闹钟模式和时钟模式切换键,同时可以使用三个功能键对闹
钟时间进行调整.同时具有串口通信功能,能够通过串口通信对时间进行修改(我定义的修改模式为
输入n后等待光标闪烁,在对nian进行修改;同理输入y,r,w,h,m,s等待1602液晶屏上对应位置的光标
闪烁后再对其进行修改).
并且本程序能将数据存入到AT2404中，再次启动电源，电子表从断电时刻开始继续计时
**********************************************************************************/
#include <reg52.h>
#include<intrins.h> 		//包含NOP空指令函数_nop_();

#define uchar unsigned char 
#define uint unsigned int 
#define AT24C02_ADD 0xa0            //AT24C02地址

sbit lcdrs=P3^5;		   //RS液晶数据命令选择端
sbit lcdrw=P3^6;		   //WR液晶读写选择端
sbit lcden=P3^4;		   //LCDEN液晶使能端
sbit dula=P2^6;			   //段选
sbit wela=P2^7;			   //位选

sbit beep=P2^3;			   //蜂鸣器
sbit led1=P1^0;			   

sbit s1=P3^0;			   //设为功能选择键
sbit s2=P3^1;  			   //设为数值增大键
sbit s3=P3^2;			   //设为数值减小键
sbit s4=P3^3;			   //模式切换键

sbit SDA=P2^0;						//时钟总线
sbit SCL=P2^1;						//数据总线

bit write=0;							//写24CO2的标志

uchar shuzi[10]="0123456789";
uchar table[14]="ALARM MODE SET";
uchar chuankou;

char tou=20,nian=16,yue=1,ri=31,week=1,hour=12,min=0,sec=0;//设置年月日时分秒位
char ialarm,ihour=12,imin=0,isec=0;//设置闹钟模式时分秒,ialarm为闹钟开关

uchar time=0,addr,alarm=0,setting=0,flag;  //addr:光标选择位,alarm:闹钟开关标志

void init();				   //定时器和1602显示屏初始化设定函数
void write_com(uchar);		   //液晶命令写入
void write_data(uchar);		   //液晶数据写入
void display(uchar,uchar);	   //液晶显示函数
void keyscan();				   //独立按键数字修改函数
void beeper();				   //蜂鸣器函数
void delayms(uchar);		   //一般延时函数
void delay();					//4us延时
void screen();				   //闹钟界面显示函数
void weekdisplay(uchar);		//星期显示
void chosealarm(uchar);			//闹钟开关显示
void chuankoutongxin();		//串口通信
void i2c_init();				//I2C初始化
void i2c_stop();			//i2c停止函数
void i2c_respons();			//应答
bit i2c_waitACK();							  //i2c应答接收判断信号
void i2c_sendbyte(uchar bt);					  //发送一字节数据
uchar i2c_recbyte();							  //接收一字节数据
char i2c_readbyte(uchar);	  //读取一字节数据
void i2c_writebyte(uchar,uchar);	  //读取一字节数据
void naozhongpanduan();			//闹钟判断


void main ()
{
	init ();				  //初始化
	i2c_init();				  //I2C初始化
	
	nian=16,yue=1,ri=31,week=1,hour=12,min=0,sec=0;//设置年月日时分秒位
	
	i2c_writebyte(1,nian);	//在24C02的地址1中写入数据nian
	i2c_writebyte(2,yue);	//在24C02的地址2中写入数据yue
	i2c_writebyte(3,ri);	//在24C02的地址3中写入数据ri
	i2c_writebyte(4,week);	//在24C02的地址4中写入数据week
	i2c_writebyte(5,hour);	//在24C02的地址5中写入数据hour
	i2c_writebyte(6,min);	//在24C02的地址6中写入数据min
	i2c_writebyte(7,sec);	//在24C02的地址7中写入数据sec
	i2c_writebyte(11,ialarm);	//在24C02的地址11中写入数据ialarm
	i2c_writebyte(12,ihour);	//在24C02的地址12中写入数据ihour
	i2c_writebyte(13,imin);	//在24C02的地址13中写入数据imin
  	
	nian=i2c_readbyte(1);	//从24C02的地址1中读出数据nian
	yue=i2c_readbyte(2);	//从24C02的地址2中读出数据yue
	ri=i2c_readbyte(3);	//从24C02的地址3中读出数据ri
	week=i2c_readbyte(4);	//从24C02的地址4中读出数据week
	hour=i2c_readbyte(5);	//从24C02的地址5中读出数据hour
	min=i2c_readbyte(6);	//从24C02的地址6中读出数据min
	sec=i2c_readbyte(7);	//从24C02的地址7中读出数据sec
	ialarm=i2c_readbyte(11);	//从24C02的地址11中读出数据ialarm
	ihour=i2c_readbyte(12);	//从24C02的地址12中读出数据ihour
	imin=i2c_readbyte(13);	//从24C02的地址13中读出数据imin

	display(0x80,tou);
	display(0x80+2,nian);
	write_data('-');
	display(0x80+5,yue);
	write_data('-');
	display(0x80+8,ri);
	weekdisplay(week);
	display(0x80+0x44,hour);
	write_data(':');
	display(0x80+0x47,min);
	write_data(':');
	display(0x80+0x4a,sec);
	
	flag=0;				  //串口通信接收标志位清零,
	while (1)			  //写在这里是因为main函数
	{					  //前赋初值无效
		if(flag==1)
		{
			TR0=0;
			chuankoutongxin();
		}

		keyscan ();
		
		naozhongpanduan();
		
		if(1==write)
		{
			write=0;
			
			i2c_writebyte(1,nian);	//在24C02的地址1中写入数据nian
			i2c_writebyte(2,yue);	//在24C02的地址2中写入数据yue
			i2c_writebyte(3,ri);	//在24C02的地址3中写入数据ri
			i2c_writebyte(4,week);	//在24C02的地址4中写入数据week
			i2c_writebyte(5,hour);	//在24C02的地址5中写入数据hour
			i2c_writebyte(6,min);	//在24C02的地址6中写入数据min
			i2c_writebyte(7,sec);	//在24C02的地址7中写入数据sec
			i2c_writebyte(11,ialarm);	//在24C02的地址11中写入数据ialarm
			i2c_writebyte(12,ihour);	//在24C02的地址12中写入数据ihour
			i2c_writebyte(13,imin);	//在24C02的地址13中写入数据imin	
		}		
	}
}

/*闹钟判断*/
void naozhongpanduan()
{
	uint i;
	if(ialarm==0&&alarm==1)
	{
		if((hour==ihour)&&(min==imin))
		{
			beep=0;			//开蜂鸣器
			for(i=0;i<600;i++)	//1min
				delayms(100);	//0.1s
			beep=1;				//关蜂鸣器
			alarm=0;			//一分钟后自动关闭闹铃
		}
	}
}

/*串口通信*/
void chuankoutongxin()
{
	if(chuankou=='n')
	{
		write_com(0x80+3);	 //光标移到年位置(第一排4位)
		write_com(0x0f);	 //光标闪烁
		
		while(!RI);
		RI=0;
		nian=SBUF;	
		display(0x80+2,nian);//年变化时重新写入	
		
		write_com(0x0c);		//开显示,但取消光标闪烁
		flag=0;					
		TR0=1;					//清零标志位
	}
	if(chuankou=='y')
	{
		write_com(0x80+6);	 //光标移到月位置(第一排7位)
		write_com(0x0f);		 //光标闪烁
	
		while(!RI);
		RI=0;
		yue=SBUF;
		display(0x80+5,yue);//月变化时重新写入	
	
		write_com(0x0c);		  //开显示,但取消光标闪烁
		flag=0;
		TR0=1;
	}
	if(chuankou=='r')
	{
		write_com(0x80+9);	 //光标移到日位置(第一排10位)
		write_com(0x0f);		 //光标闪烁
				
		while(!RI);
		RI=0;
		ri=SBUF;
		display(0x80+8,ri);//日变化时重新写入	

		write_com(0x0c);		  //开显示,但取消光标闪烁
		flag=0;
		TR0=1;
	}
	if(chuankou=='w')
	{
		write_com(0x80+15);	 //光标移到星期位置(第一排16位)
		write_com(0x0f);		 //光标闪烁
			
		while(!RI);
		RI=0;
		week=SBUF;
		weekdisplay(week);	

		write_com(0x0c);		  //开显示,但取消光标闪烁
		flag=0;
		TR0=1;
	}
	if(chuankou=='h')
	{
		write_com(0x80+0x45);	 //光标移到时位置(第一排16位)
		write_com(0x0f);		 //光标闪烁
			
		while(!RI);
		RI=0;
		hour=SBUF;
		display(0x80+0x44,hour);//星期变化时重新写入	
	
		write_com(0x0c);		  //开显示,但取消光标闪烁
		flag=0;
		TR0=1;
	}
	if(chuankou=='m')
	{
		write_com(0x80+0x48);	 //光标移到星期位置(第一排16位)
		write_com(0x0f);		 //光标闪烁
			
		while(!RI);
		RI=0;
		min=SBUF;
		display(0x80+0x47,min);//星期变化时重新写入	
	
		write_com(0x0c);		  //开显示,但取消光标闪烁
		flag=0;
		TR0=1;
	}		   
	if(chuankou=='s')
	{
		write_com(0x80+0x4b);	 //光标移到星期位置(第一排16位)
		write_com(0x0f);		 //光标闪烁		
						
		while(!RI);
		RI=0;
		sec=SBUF;
		display(0x80+0x4a,sec);//星期变化时重新写入	
	
		write_com(0x0c);		  //开显示,但取消光标闪烁
		flag=0;
		TR0=1;
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
		write=1;				  //一秒钟写入一次AT24C02
		if (sec==60)			  //1min
		{
			sec=0;
			min++;
			if (min==60)		  //1h
			{
				min=0;
				hour++;
				if (hour==24)
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
							display(0x80+2,nian);//年变化则重新写入年
						}
						display(0x80+5,yue);//月变化则重新写月
					}
					display(0x80+0x8,ri);//日变化则重新写入日
					weekdisplay(week);//周变化则重新写入周
				}
				display(0x80+0x44,hour);//时变化则重新写入时
			}
			display(0x80+0x47,min);//分变化则重新写入分
		}
		display(0x80+0x4a,sec);//秒变化则重新写入秒
	}
}

/*串口中断服务*/
void ser() interrupt 4
{
	RI=0;					//软件清楚接收中断
	chuankou=SBUF;		//把数据存入到数据数组
	flag=1;					//标志是否已收到数据
} 

/*定时器和1602显示屏初始化设定函数*/
void init ()
{
	dula=0;		 	
	wela=0;			//关闭段选和位选,防止数码管点亮
	lcden=0;		//关闭液晶使能端

	TMOD=0X21;					  //设置定时器1的工作方式为2,定时器0工作方式1
	TH0=(65536-45872)/256;		  //50ms
	TL0=(65536-45872)%256;
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
		for (j=114;j>0;j--);   //1ms

}

/*4us延时*/
void delay()
{
	_nop_();
	_nop_();
	_nop_();
	_nop_();
}

/*独立按键数字修改函数*/
void keyscan ()
{
	if(s4!=0)
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
					write_com(0x80+15);	 	//光标移到星期位置(第二排16位)
					write_com(0x0f);		 //光标闪烁
				}
				if (addr==5)
				{
					write_com(0x80+9);	 //光标移到日位置(第二排10位)
					write_com(0x0f);		 //光标闪烁
				}
				if (addr==6)
				{
					write_com(0x80+6);	 //光标移到月位置(第二排7位)
					write_com(0x0f);		 //光标闪烁
				}
				if (addr==7)
				{
					write_com(0x80+3);	 //光标移到年位置(第二排4位)
					write_com(0x0f);		 //光标闪烁
				}
				if (addr==8)
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
					i2c_writebyte(7,sec);	//在24C02的地址7中写入数据sec	
				}
				if (addr==2) 	 //光标选择位在分钟位
				{
					min++;
					if (min==60)
						min=0;	
					display(0x80+0x47,min);//分变化时重新写入
					write_com(0x80+0x48);//写入后将光标移回并闪烁，显示调整状态
					i2c_writebyte(6,min);	//在24C02的地址6中写入数据min	
				}
				if (addr==3) 	 //光标选择位在小时位
				{
					hour++;
					if (hour==24)
						hour=0;
					display(0x80+0x44,hour);//时变化时重新写入	
					write_com(0x80+0x45);//写入后将光标移回并闪烁，显示调整状态
					i2c_writebyte(5,hour);	//在24C02的地址5中写入数据hour	
				}
				if (addr==4) 	 //光标选择位在星期位
				{
					week++;
					if (week==8)
						week=1;
					weekdisplay(week);	
					write_com(0x80+15);//写入后将光标移回并闪烁，显示调整状态
					i2c_writebyte(4,week);	//在24C02的地址4中写入数据week	
				}
				if (addr==5) 	 //光标选择位在日位
				{
					ri++;
					if (ri==32)
						ri=1;
					display(0x80+8,ri);//时变化时重新写入	
					write_com(0x80+9);//写入后将光标移回并闪烁，显示调整状态
					i2c_writebyte(3,ri);	//在24C02的地址3中写入数据ri	
				}
				if (addr==6) 	 //光标选择位在小时位
				{
					yue++;
					if (yue==13)
						yue=1;
					display(0x80+5,yue);//时变化时重新写入	
					write_com(0x80+6);//写入后将光标移回并闪烁，显示调整状态
					i2c_writebyte(2,yue);	//在24C02的地址2中写入数据yue	
				}
				if (addr==7) 	 //光标选择位在小时位
				{
					nian++;
					if (nian==100)
						hour=0;
					display(0x80+2,nian);//时变化时重新写入	
					write_com(0x80+3);//写入后将光标移回并闪烁，显示调整状态
					i2c_writebyte(1,nian);	//在24C02的地址1中写入数据nian	
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
					i2c_writebyte(7,sec);	//在24C02的地址7中写入数据sec	
				}
				if (addr==2) 
				{
					min--;
					if (min==-1)
						min=59;
					display(0x80+0x47,min);//分变化时重新写入
					write_com(0x80+0x48);//写入后将光标移回并闪烁，显示调整状态
					i2c_writebyte(6,min);	//在24C02的地址6中写入数据min	
				}
				if (addr==3) 
				{
					hour--;
					if (hour==-1)
						hour=23;
					display(0x80+0x44,hour);//时变化时重新写入
					write_com(0x80+0x45);//写入后将光标移回并闪烁，显示调整状态
					i2c_writebyte(5,hour);	//在24C02的地址5中写入数据hour	
				}
				if (addr==4) 
				{
					week--;
					if (week==0)
						week=7;
					weekdisplay(week);
					write_com(0x80+15);//写入后将光标移回并闪烁，显示调整状态
					i2c_writebyte(4,week);	//在24C02的地址4中写入数据week	
				}
				if (addr==5) 
				{
					ri--;
					if (ri==0)
						ri=31;
					display(0x80+8,ri);//日变化时重新写入
					write_com(0x80+9);//写入后将光标移回并闪烁，显示调整状态
					i2c_writebyte(3,ri);	//在24C02的地址3中写入数据ri	
				}
				if (addr==6) 
				{
					yue--;
					if (yue==0)
						yue=12;
					display(0x80+5,yue);//月变化时重新写入
					write_com(0x80+6);//写入后将光标移回并闪烁，显示调整状态
					i2c_writebyte(2,yue);	//在24C02的地址2中写入数据yue	
				}
				if (addr==7) 
				{
					nian--;
					if (nian==-1)
						nian=99;
					display(0x80+2,nian);//年变化时重新写入
					write_com(0x80+3);//写入后将光标移回并闪烁，显示调整状态
					i2c_writebyte(1,nian);	//在24C02的地址1中写入数据nian	
				}
			}
		}
	}
	/*判断是否进入闹钟模式*/
	if(s4==0)
	{
		delayms (10);		   //延时防抖
		if(s4==0)			   //重复判断,防抖
		{
			alarm=1;
			setting=1;
			beeper();		   //按键按下时蜂鸣器响,作为反馈
			led1=0;			   //开启闹钟模式,亮灯反馈
			delayms(200);	   //延时
			led1=1;
			TR0=0;
			screen();		   //进入闹钟模式界面
			
			do
			{
				if (s1==0)
				{
					delayms (10);		   //延时防抖
					if (s1==0)			   //重复判断,防抖
					{
												addr++;		   //液晶选择位变化
						beeper();	   //按键按下时蜂鸣器响0.1s,作为反馈
						while (!s1);   //等待按键恢复
						
						if(addr==1)
						{
							write_com(0x80+0x40+2);	 //光标移到时位置(第二排7位)
							write_com(0x0f);		 //光标闪烁	
						}
						if(addr==2)
						{
							write_com(0x80+0x40+7);	 //光标移到时位置(第二排7位)
							write_com(0x0f);		 //光标闪烁	
						}						
						if (addr==3)					 
						{
							write_com(0x80+0x40+10);  //光标移到分位置(第二排11位)
							write_com(0x0f);		  //光标闪烁
						}
						if (addr==4)
						{
							addr=0;				  	  //光标选择位清零(取消选择)
							write_com(0x0c);		  //开显示,但取消光标闪烁
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
						if (addr==1)	//光标选择位在闹钟开关选择位
						{
							ialarm++;			 
							if (ialarm==2)
								ialarm=0;
							chosealarm(ialarm);
							write_com(0x80+0x40+2);//写入后将光标移回并闪烁，显示为调整状态
							i2c_writebyte(11,ialarm);	//在24C02的地址1中写入数据ialarm	
						}
						if (addr==2)	//光标选择位在时位
						{
							ihour++;			 
							if (ihour==24)
								ihour=0;
							display(0x80+0x40+6,ihour);//时变化时重新写入
							write_com(0x80+0x40+7);//写入后将光标移回并闪烁，显示为调整状态
							i2c_writebyte(12,ihour);	//在24C02的地址12中写入数据ihour	
						}
						if (addr==3) 	 //光标选择位在分钟位
						{
							imin++;
							if (imin==60)
								imin=0;	
							display(0x80+0x40+9,imin);//分变化时重新写入
							write_com(0x80+0x40+10);//写入后将光标移回并闪烁，显示调整状态
							i2c_writebyte(13,imin);	//在24C02的地址1中写入数据imin	
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
							ialarm--;
							if (ialarm==-1)
								ialarm=1;
							chosealarm(ialarm);
							write_com(0x80+0x40+2);//写入后将光标移回并闪烁，显示调整状态
							i2c_writebyte(11,ialarm);	//在24C02的地址1中写入数据ialarm	
						}
						if (addr==2)
 						{
							ihour--;
							if (ihour==-1)
								ihour=23;
							display(0x80+0x40+6,ihour);//时变化时重新写入
							write_com(0x80+0x40+7);//写入后将光标移回并闪烁，显示调整状态
							i2c_writebyte(12,ihour);	//在24C02的地址12中写入数据ihour	
						}
						if (addr==3) 
						{
							imin--;
							if (imin==-1)
								imin=59;
							display(0x80+0x40+9,imin);//分变化时重新写入
							write_com(0x80+0x40+10);//写入后将光标移回并闪烁，显示调整状态
							i2c_writebyte(13,imin);	//在24C02的地址1中写入数据imin	
						}
					}
				}
				
				if(s4==0)
				{
					delayms (10);		   //延时防抖
					if(s4==0)			   //重复判断,防抖
					{
						beeper();		   //按键按下时蜂鸣器响,作为反馈
						led1=0;			   //开启闹钟模式,亮灯反馈
						delayms(200);	   //延时
						led1=1;
						TR0=1;
						setting=0;

						write_com(0x01);		 //00000001B,显示清0，数据指针清0

						display(0x80,tou);
						display(0x80+2,nian);
						write_data('-');
						display(0x80+5,yue);
   						write_data('-');
						display(0x80+8,ri);
						weekdisplay(week);
						display(0x80+0x44,hour);
						write_data(':');
						display(0x80+0x47,min);
						write_data(':');
						display(0x80+0x4a,sec);
					}
				}
			}while(setting);
		}
	}
}

/*闹钟模式显示*/
void screen()
{
	uchar i;

	write_com(0x01);		 //00000001B,显示清0，数据指针清0

	write_com (0x80+0x01);
	for (i=0;i<14;i++)
	{
		write_data(table[i]);	
	}
	write_com(0x80+0x40+1);
	chosealarm(ialarm);
	display(0x80+0x40+6,ihour);
	write_data(':');
	display(0x80+0x40+9,imin);	
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

/*星期几显示函数*/
void weekdisplay(uchar a)
{
	switch(a)
	{
		case 1:	write_com(0x80+13);
				write_data('M');
				write_data('0');
				write_data('N');
				break;
		case 2:	write_com(0x80+13);
				write_data('T');
				write_data('U');
				write_data('E');
				break;
		case 3:	write_com(0x80+13);
				write_data('W');
				write_data('E');
				write_data('D');
				break;
		case 4:	write_com(0x80+13);
				write_data('T');
				write_data('H');
				write_data('U');
				break;
		case 5:	write_com(0x80+13);
				write_data('F');
				write_data('R');
				write_data('I');
				break;
		case 6:	write_com(0x80+13);
				write_data('S');
				write_data('A');
				write_data('T');
				break;
		case 7:	write_com(0x80+13);
				write_data('S');
				write_data('U');
				write_data('N');
				break;
	}
}

/*闹钟模式选择函数*/
void chosealarm(uchar b)
{
	if(0==b)
	{
		write_com(0x80+0x40+1);
		write_data('O');
		write_data('N');
		write_data(' ');
	}
	if(1==b)
	{
		write_com(0x80+0x40+1);
		write_data('O');
		write_data('F');
		write_data('F');
	}
}		

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
char i2c_readbyte(uchar address)
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
void i2c_writebyte(uchar address, char dat)
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