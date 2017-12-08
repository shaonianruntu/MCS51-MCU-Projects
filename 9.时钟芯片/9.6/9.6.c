#include<reg52.h>
#include<string.h>
#define uint unsigned int
#define uchar unsigned char

sbit s1=P3^0;		//S1设为功能选择键
sbit s2=P3^1;		//S2设为数值增大键
sbit s3=P3^2;		//S3设为数字减小键
sbit s4=P3^3;		//S4设为时钟/闹钟切换键
//管脚定义
sbit SCLK=P1^0;    	//DS1302时钟信号  7脚       
sbit DIO=P1^1;      //DS1302数据信号  6脚     
sbit CE=P1^2;  		//DS1302片选	  5脚
//位寻址寄存器定义              
sbit ACC0=ACC^0;	//ACC累加器(ACC:8位寄存器)的D0位
sbit ACC7=ACC^7;	//ACC累加器的D7位

sbit dula=P2^6;		//段选
sbit wela=P2^7;		//位选
sbit beep=P2^4;		//蜂鸣器

sbit lcdrw=P3^6;	//1602液晶读写选择端
sbit lcdrs=P3^5;	//1602数据命令选择端
sbit lcden=P3^4;	//1602使能端

uchar hang1[16]={"2016-01-31      "};
uchar hang2[16]={"TIME 00:00:00   "};


void write_com(uchar);		//液晶命令写入
void write_data(uchar);		//液晶数据写入
void display(void);				//液晶显示
void delayms(uint);			//ms级延时函数
void init(void);				//1602液晶初始化
void write_1302(uchar,uchar);//DS1302:写入数据(先送地址，再写数据)
uchar read_1302(uchar);		//DS1302:读取数据(先送地址，再读数据)
void write_byte(uchar); 	//S1302：写入操作(上升沿)
uchar read_byte(void);		//DS1302：读取操作(下降沿)
void huoqu(uchar);			//日期获取函数
void week(uchar);			//星期几获取函数
void keyscan(void);				//独立按键数字修改函数
void beeper(void);				//蜂鸣器函数
void setdisplay(uchar,uchar);//修改状态下个位位液晶重新显示

void main()
{
	uchar i,temp;
	uchar clock[7];//秒,分,时,日,月,周,年寄存器初始值
	init();	
	temp=0x80;			    //写寄存器的地址为0x80
	write_1302(0x8e,0x00);			//WP=0写操作
	for(i=0;i<7;i++)
	{
	 	write_1302(temp,clock[i]);
		temp+=2;					//偶数位
	}
	write_1302(0x8e,0x80);			//写保护
	while(1)
	{
		huoqu(clock);
		display();
		temp=0x81;					//读寄存器的地址
		for(i=0;i<7;i++)
		{
			clock[i]=read_1302(temp);
			temp+=2;
		}							//奇数位
	 }
}

void huoqu(uchar *poi)
{
	/*秒获取*/
	hang2[11]=poi[0]/16+'0';
	hang2[12]=poi[0]%16+'0';
	/*分获取*/
	hang2[8]=poi[1]/16+'0';
	hang2[9]=poi[1]%16+'0';
	/*小时获取*/
	hang2[5]=poi[2]/16+'0';
	hang2[6]=poi[2]%16+'0';
	/*日获取*/
	hang1[8]=poi[3]/16+'0';
	hang1[9]=poi[3]%16+'0';
	/*月获取*/
	hang1[5]=poi[4]/16+'0';
	hang1[6]=poi[4]%16+'0';
	/*年获取*/
	hang1[2]=poi[6]/16+'0';
	hang1[3]=poi[6]%16+'0';
		
	week(poi[5]);	//周获取
}

/*周获取*/
void week(uchar day)
{
	switch(day)
	{
		case 1:	hang1[13]='S';	hang1[14]='U';	
				hang1[15]='N';	break;
		case 2:	hang1[13]='M';	hang1[14]='O';	
				hang1[15]='N';	break;
		case 3:	hang1[13]='T';	hang1[14]='U';	
				hang1[15]='S';	break;
		case 4:	hang1[13]='W';	hang1[14]='E';	
				hang1[15]='N';	break;
		case 5:	hang1[13]='T';	hang1[14]='H';	
				hang1[15]='U';	break;
		case 6:	hang1[13]='F';	hang1[14]='R';	
				hang1[15]='I';	break;
		case 7:	hang1[13]='S';	hang1[14]='A';	
				hang1[15]='T';	break;

	}
}

/*液晶整行显示*/
void display(void)
{
	uchar i;
	/*1602第一行显示*/
	write_com(0x80);
	for(i=0;i<16;i++)
	{
		write_data(hang1[i]);
	}
	/*1602第二行显示*/
	write_com(0x80+0x40);
	for(i=0;i<16;i++)
	{
		write_data(hang2[i]);
	}	
}

/*液晶命令写入*/
void write_com(uchar com)
{
	lcdrw=0;		//选择写
	lcdrs=0;		//选择命令
	P0=com;
	delayms(5);		//延时使液晶运行稳定
	lcden=1;		//高脉冲将数据送入液晶控制器
	delayms(5);
	lcden=0;	
}

/*液晶数据写入*/
void write_data(uchar date)
{
	lcdrw=0;		//选择写
	lcdrs=1;		//选择数据
	P0=date;
	delayms(5);		//延时使液晶运行稳定
	lcden=1;		//高脉冲将数据送入液晶控制器
	delayms(5);
	lcden=0;
}

/*ms级延时函数*/
void delayms(uint xms)
{
	uint i,j;
	for(i=xms;i>0;i--)
		for(j=114;j>0;j--);
}

/*1602液晶初始化*/
void init(void)
{
	dula=0;
	wela=0;		//关闭段选和位选,防止数码管点亮
	lcden=0;	//关闭液晶使能端

	write_com(0x38);		 //00111000B,设置为16*2显示，5*7点阵，8位数据接口
	write_com(0x0c);		 //00001100B,设置开显示，不显示光标
	write_com(0x06);		 //00000110B,写一个字符后地址指针加1
	write_com(0x01);		 //00000001B,显示清0，数据指针清0
}

/*DS1302：写入操作(上升沿)*/ 
void write_byte(uchar date)
{
   uchar i;
   ACC=date;			 //将要写入的数据放入ACC
   for(i=8;i>0;i--)		 //由低到高写入8位数据
   { 
      DIO=ACC0;			 //每次传输低字节
	  SCLK=0;     		 
      SCLK=1;			 //在时钟的上升沿写入一位数据
      ACC=ACC>>1;		 //右移一位,将高一位移入ACC0
   }
}

/*DS1302：读取操作(下降沿)*/
uchar read_byte(void)
{
   uchar i;
   for(i=0;i<8;i++)		 //由低到高读出8位数据
   {
      ACC=ACC>>1;		 //将前一下降沿读出的数据右移1位,从而使该次读出的数据放入ACC7
	  SCLK = 1;
	  SCLK = 0;			 //在时钟的下降沿读出1位数据
      ACC7 = DIO;		 
   }
   return(ACC);
}

/*DS1302:写入数据(先送地址，再写数据)*/ 
void write_1302(uchar addr,uchar date)
{
   	CE=0;    			//CE引脚为低,数据传送中止,停止工作
	SCLK=0;              //清零时钟总线                   
    CE=1;  			    //CE引脚为高,逻辑控制有效,重新工作
    write_byte(addr);    //写入地址
    write_byte(date);	//写入数据
    CE=0;				//CE引脚为低,数据传送中止,停止工作
    SCLK=1;
}

/*DS1302:读取数据(先送地址，再读数据)*/
uchar read_1302(uchar addr)
{
    uchar temp;
    CE=0;                 //CE引脚为低,数据传送中止,停止工作    
    SCLK=0;  			 //清零时钟总线 
    CE=1;                 //CE引脚为高,逻辑控制有效,重新工作
    write_byte(addr);     //写入地址
    temp=read_byte();	 //读取数据
    CE=0;				 //CE引脚为低,数据传送中止,停止工作
    SCLK=1;               
    return(temp);
}

/*独立按键数字修改函数*/
/*
void keyscan (void)
{
	uchar addr;	//光标位
	if(s4!=0)	//没有进入闹钟界面
	{
		if (s1==0)		 //s1按键,选择需要设置的位
		{
			delayms (10);		//延时防抖
			if (s1==0)			//重复判断,防抖
			{
				addr++;		   //液晶选择位变化
				beeper();	   //按键按下时蜂鸣器响0.1s,作为反馈
				while (!s1);   //等待按键恢复
									
				if (addr==1)
				{
					TR0=0;
					write_com(0x80+0x40+12);	 //光标移到秒位置(第二排13位)
					write_com(0x0f);	 //光标闪烁
				}
				if (addr==2)
				{
					write_com(0x80+0x40+9);	 //光标移到分位置(第二排10位)
					write_com(0x0f);	 //光标闪烁
				}
				if (addr==3)					 
				{
					write_com(0x80+0x40+6);	  //光标移到时位置(第二排7位)
					write_com(0x0f);	  //光标闪烁
				}
				if (addr==4)					 
				{
					write_com(0x80+13);	  //光标移到星期位置(第一排14位)
					write_com(0x0f);	  //光标闪烁
				}
				if(addr==5)
				{
					write_com(0x80+9);	  //光标移到日位置(第一排10位)
					write_com(0x0f);	  //光标闪烁
				}
				if(addr==6)
				{
					write_com(0x80+6);	  //光标移到月位置(第一排7位)
					write_com(0xf0);	  //光标闪烁
				}
				if(addr==7)
				{
					write_com(0x80+3);	  //光标移到年位置(第一排4位)
					write_com(0xf0);	  //光标闪烁
				}
				if (addr==8)
				{
					addr=0;				  	  //光标选择位清零(取消选择)
					write_com(0x0c);		  //开显示,但取消光标闪烁
					TR0=1;					  //启动时钟,表开始走
				}
			}	
		}
		if(s2==0&&addr!=0)//只有功能键被按下之后数值键才起作用
		{
			delayms (10);			//延时防抖
			if (s2==0)				//重复判断,防抖
			{	
				beeper ();		//按键按下时蜂鸣器响,作为反馈
				while (!s2);	//等待按键恢复
				if (addr==1)	//光标选择位在秒位
 				{
					clock[0]++;			 
					if (clock[0]==60)
						clock[0]=0;
					setdisplay(0x80+0x40+12,clock[0]);//秒变化时重新写入
					write_com(0x80+0x40+12);//写入后将光标移回并闪烁，显示为调整状态
				}
			
				if (addr==2) 	 //光标选择位在分钟位
				{
					clock[1]++;
					if (clock[1]==60)
						clock[1]=0;	
					setdisplay(0x80+0x40+9,clock[1]);//分变化时重新写入
					write_com(0x80+0x40+9);//写入后将光标移回并闪烁，显示调整状态
				}

				if (addr==3) 	 //光标选择位在小时位
				{
					clock[2]++;
					if (clock[2]==24)
						clock[2]=0;
					setdisplay(0x80+0x40+6,clock[2]);//时变化时重新写入	
					write_com(0x80+0x40+6);//写入后将光标移回并闪烁，显示调整状态
				}
				
				if(addr==4)		  //光标选择位在星期位
				{
					clock[5]++;
					if(clock[5]==8)
						clock[5]=1;
					display();			  //星期变化后重新写入
					write_com(0x80+13);	  //写入后将光标移回并闪烁,显示调整状态
				}

				if(addr==5)		 //光标选择位在日位
				{
					clock[3]++;
					if(clock[3]==32)
						clock[3]=1;
					setdisplay(0x80+9,clock[3]);   //日变化后重新写入
					write_com(0x80+9);	  //写入后将光标移回并闪烁,显示调整状态
				}

				if(addr==6)		 //光标选择位在月位
				{
					clock[4]++;
					if(clock[4]==13)
						clock[4]=1;
					setdisplay(0x80+6,clock[4]);  //月变化后重新写入
					write_com(0x80+6);	  //写入后将光标移回并闪烁,显示调整状态
				}

				if(addr==7)
				{
					clock[6]++;
					if(clock[6]==100)
						clock[6]=0;
					setdisplay(0x80+3,clock[6]); //年变化时重新输入
					write_com(0x80+3);	  //写入后将光标移回并闪烁,显示调整状态
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
				if (addr==1)	//光标选择位在秒位
 				{
					if (clock[0]==0)
						clock[0]=60;
					clock[0]--;			 
					setdisplay(0x80+0x40+12,clock[0]);//秒变化时重新写入
					write_com(0x80+0x40+12);//写入后将光标移回并闪烁，显示为调整状态
				}
				
				if (addr==2) 	 //光标选择位在分钟位
				{
					if (clock[1]==0)
						clock[1]=60;
					clock[1]--;
					setdisplay(0x80+0x40+9,clock[1]);//分变化时重新写入
					write_com(0x80+0x40+9);//写入后将光标移回并闪烁，显示调整状态
				}

				if (addr==3) 	 //光标选择位在小时位
				{
					if (clock[2]==0)
						clock[2]=24;
					clock[2]--;
					setdisplay(0x80+0x40+6,clock[2]);//时变化时重新写入	
					write_com(0x80+0x40+6);//写入后将光标移回并闪烁，显示调整状态
				}
				
				if(addr==4)		  //光标选择位在星期位
				{
					if(clock[5]==1)
						clock[5]=8;
					clock[5]--;
					display();			  //星期变化后重新写入
					write_com(0x80+13);	  //写入后将光标移回并闪烁,显示调整状态
				}

				if(addr==5)		 //光标选择位在日位
				{
					if(clock[3]==1)
						clock[3]=32;
					clock[3]--;
					setdisplay(0x80+9,clock[3]);   //日变化后重新写入
					write_com(0x80+9);	  //写入后将光标移回并闪烁,显示调整状态
				}

				if(addr==6)		 //光标选择位在月位
				{
					if(clock[4]==1)
						clock[4]=13;
					clock[4]--;
					setdisplay(0x80+6,clock[4]);  //月变化后重新写入
					write_com(0x80+6);	  //写入后将光标移回并闪烁,显示调整状态
				}

				if(addr==7)
				{
					if(clock[6]==0)
						clock[6]=100;
					clock[6]--;
					setdisplay(0x80+3,clock[6]); //年变化时重新输入
					write_com(0x80+3);	  //写入后将光标移回并闪烁,显示调整状态
				}
			}
		}
	}	
}
*/

/*蜂鸣器函数*/
void beeper(void)
{
	beep=0;				//开启蜂鸣器
	delayms(100);		//0.1s延时
	beep=1;				//关闭蜂鸣器
}

/*液晶修改状态下显示函数*/
void setdisplay(uchar add,uchar date)
{
	uchar shi,ge;
	shi=date/16+'0';	
	ge=date%16+'0';

	write_com(add);
	write_data(shi);
	write_data(ge);
}

