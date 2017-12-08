#include <reg52.h>
#include <intrins.h>
#include <string.h>
#define uint unsigned int
#define uchar unsigned char
//定义DS1302与MCU接口
sbit SCLK = P1^0;//时钟线引脚
sbit IO = P1^1;//IO接口
sbit RST = P1^2;//复位线引脚
//定义1602与单片机的接口
sbit RS = P3^5;//指令数据选择
sbit RW = P3^6;//接地，执行写操作
sbit EN = P3^4;//信号使能

uchar *WEEK[]=
{
 	"SUN","MON","TUS","WEN","THU","FRI","SAT"
};
uchar LCD_DSY_BUFFER1[]={"DATE 00-00-00    "};
uchar LCD_DSY_BUFFER2[]={"TIME 00:00:00    "};
uchar DateTime[7];

/********延时i毫秒函数************/
void DelayMS(uint ms)
{
 	uchar i;
	while(ms--)
	{
	 	for(i=0;i<120;i++);
	}
}
/*****向DS1302写入1字节数据**********/
void Write_A_Byte_TO_DS1302(uchar x)
{
 	uchar i;
	for(i=0;i<8;i++)
	{
	 	IO=x&0x01;SCLK=1;SCLK=0;
                x>>=1;//x右移1位，高位补0
	}
}
/*****从DS1302中读出1字节数据**********/
uchar Get_A_Byte_FROM_DS1302()
{
 	uchar i,b=0x00;
	for(i=0;i<8;i++)
	{
	 	b=b|_crol_((uchar)IO,i);
		SCLK=1;SCLK=0;
	}
	return b/16*10+b%16;
}
/***********读取DS1302某地址的数据，先写命令字，后读数据************/
uchar Read_Data(uchar addr)
{
 	uchar dat;
	RST = 0;//复位
	SCLK=0;//时钟脉冲置0
        RST=1;//启动数据传送
   	Write_A_Byte_TO_DS1302(addr);//写入地址命令字
	dat = Get_A_Byte_FROM_DS1302();//读出一个字节的数据
   	SCLK=1;RST=0;
	return dat;
}

void GetTime()
{
 	uchar i,addr=0x81;
	for(i=0;i<7;i++)
	{
	 	DateTime[i]=Read_Data(addr);addr+=2;
	}
}
/*************读出LCD状态*************/
uchar Read_LCD_State()
{
 	uchar state;
	RS=0;RW=1;EN=1;DelayMS(1);
	state=P0;
	EN = 0;DelayMS(1);
	return state;
}

/***********LCD忙检测*****************/
void LCD_Busy_Wait()
{
 	while((Read_LCD_State()&0x80)==0x80);//读取忙标志位BF,BF=1则一直等待
	DelayMS(5);
}
/**************向LCD写数据*******************/
void Write_LCD_Data(uchar dat)
{
 	LCD_Busy_Wait();//忙检测确保上一指令完成，也可用适当的延时替换此行
	RS=1;RW=0;EN=0;P0=dat;EN=1;DelayMS(1);EN=0;	
}

void Write_LCD_Command(uchar cmd)
{
 	LCD_Busy_Wait();
	RS=0;RW=0;EN=0;P0=cmd;EN=1;DelayMS(1);EN=0;	
}
/*****************初始化LCD函数*****************/
void Init_LCD()
{
 	Write_LCD_Command(0x38);//8位数据接口，2行显示，5*7点阵字符
	DelayMS(1);	//延时保证上一指令完成
	Write_LCD_Command(0x01);//清DDRAM和AC值
	DelayMS(1);	
	Write_LCD_Command(0x06);//数据读写操作画面不动，AC自动加1
	DelayMS(1);	
	Write_LCD_Command(0x0c);//开显示，关光标和闪烁
	DelayMS(1);	
}

void Set_LCD_POS(uchar p)
{
 	Write_LCD_Command(p|0x80);	
}

void Display_LCD_String(uchar p,uchar *s)
{
 	uchar i;
	Set_LCD_POS(p);
	for(i=0;i<16;i++)
	{
		Write_LCD_Data(s[i]);
		DelayMS(1); 	
	}
}
/******************格式化日期时间函数**********/
void Format_DateTime(uchar d,uchar *a)
{
 	a[0]=d/10+'0';
	a[1]=d%10+'0';
}
/***************主函数***************/
void main()
{
 	Init_LCD();//初始化液晶
	while(1)
	{
	 	GetTime();//获得当前时间
		Format_DateTime(DateTime[6],LCD_DSY_BUFFER1+5);//通道号显示
		Format_DateTime(DateTime[4],LCD_DSY_BUFFER1+8);
		Format_DateTime(DateTime[3],LCD_DSY_BUFFER1+11);

		strcpy(LCD_DSY_BUFFER1+13,WEEK[DateTime[5]]);

		Format_DateTime(DateTime[2],LCD_DSY_BUFFER2+5);
		Format_DateTime(DateTime[1],LCD_DSY_BUFFER2+8);
		Format_DateTime(DateTime[0],LCD_DSY_BUFFER2+11);

		Display_LCD_String(0x00,LCD_DSY_BUFFER1);//液晶显示
		Display_LCD_String(0x40,LCD_DSY_BUFFER2);
	}
}
