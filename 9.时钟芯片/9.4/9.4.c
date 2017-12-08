#include <reg52.h>
#include <intrins.h>
#include <string.h>
#define uint unsigned int
#define uchar unsigned char
sbit dula=P2^6;
sbit wela=P2^7;
//定义DS1302与MCU接口
sbit SCLK=P1^0;      //DS1302时钟信号  7脚       
sbit IO=P1^1;        //DS1302数据信号  6脚     
sbit CE=P1^2;  		 //DS1302片选	   5脚
//位寻址寄存器定义              
sbit ACC0=ACC^0;		  //ACC累加器(ACC:8位寄存器)的D0位
sbit ACC7=ACC^7;		  //ACC累加器的D7位
//定义1602与单片机的接口
sbit lcden=P3^4;	//LCDEN液晶使能端
sbit lcdrs=P3^5;	//RS液晶数据命令选择端
sbit lcdwr=P3^6;	//WR液晶读写选择端

uchar hang1[]={"2000-00-00       "};//1602第一行
uchar hang2[]={"TIME 00:00:00    "};//1602第二行
uchar *week[]={"SUN","MON","TUS","WEN","THU","FRI","SAT"};
uchar datatime[7];

void delayms(uint xms);		  //ms级延时函数
void write_byte(uchar date);  //DS1302：写入操作(上升沿)
uchar read_byte(void);		  //DS1302：读取操作(下降沿)
void write_1302(uchar addr,uchar date);	   //DS1302:写入数据(先送地址，再写数据)
uchar read_1302(uchar addr);			   //DS1302:读取数据(先送地址，再读数据)
void write_com (uchar com);				   //液晶命令写入
void write_data(uchar date);			   //液晶数据写入
void lcd_init();						   //lcd初始化函数
void gettime();							   //获取当前时间
void display(uchar add,uchar date);        //液晶显示函数
void format_datatime(uchar *add,uchar date);//格式化日期函数


/*主函数*/
void main()
{
	lcd_init();		//初始化液晶
	while(1)
	{
	 	uchar i;
		gettime();//获得当前时间

		/*1602第一行数据写入*/
		format_datatime(hang1+2,datatime[6]);	 //年
		format_datatime(hang1+5,datatime[4]);//月		//月
		format_datatime(hang1+8,datatime[3]);//日		//日
		
		strcpy(hang1+13,week[datatime[5]]);	 //星期
				
		/*1602第二行数据写入*/
		format_datatime(hang2+5,datatime[2]); //时		
		format_datatime(hang2+8,datatime[1]); //分
		format_datatime(hang2+11,datatime[0]);//秒

		write_com(0x80);
		for(i=0;i<16;i++)
		{
			write_data(hang1[i]);
			delayms(1);
		}
		write_com(0x80+0x40);
		for(i=0;i<16;i++)
		{
			write_data(hang2[i]);
			delayms(1);
		}
		
		/*1602液晶显示*/
//		display(0x80,hang1);
//		display(0x80+0x40,hang2);
	}
}

/*lcd初始化函数*/
void lcd_init()
{
 	dula=0;			
	wela=0;			//关闭段选和位选,防止数码管点亮
	lcden=0;		//关闭液晶使能端
	
	write_com(0x38);//8位数据接口，2行显示，5*7点阵字符
	write_com(0x0c);//开显示，关光标和闪烁
	write_com(0x06);//数据读写操作画面不动，AC自动加1
	write_com(0x01);//清DDRAM和AC值
}

/*格式化日期函数*/
void format_datatime(uchar *add,uchar date)
{
	add[0]=date/10+'0';
	add[1]=date%10+'0';
}

/*液晶命令写入*/
void write_com (uchar com)
{
	lcdwr=0;		   //选择写
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
	lcdwr=0;		  //选择写
	lcdrs=1;		  //选择数据
	P0=date;		  
	delayms (5);	  //延时使液晶运行稳定
	lcden=1;		  //高脉冲将数据送入液晶控制器
	delayms (5);
	lcden=0;
}

/*液晶显示函数*/
void display(uchar add,uchar *hang)//显示时分秒函数。add什么位置 date显示什么
{
	uchar i;
	write_com(add);
	for(i=0;i<16;i++)
	{
		write_data(hang[i]);
		delayms(1);
	}
}

/*获取当前时间*/
void gettime()
{
 	uchar i;
	uchar temp=0x81;						//读寄存器地址0x81
	for(i=0;i<7;i++)
	{
	 	datatime[i]=read_1302(temp);
		temp+=2;					//奇数位读取
	}
}

/*ms级延时函数*/
void delayms(uint xms)
{
 	uint i,j;
	for(i=xms;i>0;i--)
		for(j=114;j>0;j--);
}

/*DS1302：写入操作(上升沿)*/ 
void write_byte(uchar date)
{
   uchar i;
   ACC=date;			 //将要写入的数据放入ACC
   for(i=8;i>0;i--)		 //由低到高写入8位数据
   { 
      IO=ACC0;			 //每次传输低字节
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
      ACC7 = IO;		 
   }
   return(ACC/16*10+ACC%16);	//2进制转10进制
}

/*DS1302:写入数据(先送地址，再写数据)*/ 
void write_1302(uchar addr,uchar date)
{
  	CE=0;    			//CE引脚为低,数据传送中止,停止工作
	SCLK=0;             //清零时钟总线                   
    CE=1;  			    //CE引脚为高,逻辑控制有效,重新工作
    write_byte(addr);   //写入地址
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








