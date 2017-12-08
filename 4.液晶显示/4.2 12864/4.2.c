#include <reg52.h>
#include <intrins.h>
#include <stdlib.h>	
#define uchar unsigned char
#define uint  unsigned int
/*****************端口定义****************************/
/*液晶*/
#define LCD_data  P0             //数据口
sbit lcden=P3^4;           //LCDEN液晶使能端
sbit lcdrs=P3^5;           //RS液晶数据命令选择端
sbit lcdwr=P3^6;		   //WR液晶读写选择端
/*数码管*/
sbit dula=P2^6;
sbit wela=P2^7;
uchar dis1[10]; 
uchar code dis1[]={"《猜灯谜》"};
uchar code dis2[]={"园外隐约闻猿叫，"};
uchar code dis3[]={"小桥星月闻萧声。"};
uchar code dis4[]={"（打一节日）"};
	
	
/******************************************************
******************主函数*******************************
******************************************************/
void main()
{
    uchar i;
    init();                 //初始化LCD             

    lcd_pos(0,0);             //设置显示位置为第二行的第1个字符
    i=0;
    while(dis1[i] != '\0')
    {
       write_data(dis1[i]);      //显示字符
       i++;
    }

 	lcd_pos(1,0);             //设置显示位置为第三行的第1个字符
    i = 0;
    while(dis2[i] != '\0')
    {
       write_data(dis2[i]);      //显示字符
       i++;
    }

	lcd_pos(2,0);             //设置显示位置为第四行的第1个字符
    i = 0;
    while(dis3[i] != '\0')
    {
       write_dat(dis3[i]);      //显示字符
       i++;
    }

	lcd_pos(3,0);             //设置显示位置为第四行的第1个字符
    i = 0;
    while(dis3[i] != '\0')
    {
       write_dat(dis3[i]);      //显示字符
       i++;
    }

    while(1);
}


/******************************************************
******************初始化*******************************
******************************************************/                                                                */
/*初始化函数*/
void init()
{ 
    wela=0;
	dula=0;
	lcden=0;		//关闭液晶使能端
	
	write_com(0x30);      //基本指令操作
    delayms(5);
    write_com(0x0C);      //显示开，关光标
    delayms(5);
    write_com(0x01);      //清除LCD的显示内容
    delayms(5);
}


/******************************************************
******************液晶显示*****************************
******************************************************/
/*液晶命令写入*/
void write_com(uchar com)
{
	lcdwr=0;
	lcdrs=0;
	lcden=0;
	P0=com;
	delayms(5);
	lcden=1;
	delayms(5);
	lcden=0;
}

/*液晶数据写入*/
void write_data(uchar date)
{
	lcdwr=0;
	lcdrs=1;
	lcden=0;
	P0=date;
	delayms(5);
	lcden=1;
	delayms(5);
	lcden=0;
}

/*液晶显示位置设置*/
void lcd_pos(uchar x,uchar y)
{                          
    uchar pos;
    switch(x)
    {
   		case 0: x=0x80; break;	 //line1
		case 1: x=0x90; break;	 //line2
		case 2: x=0x88; break;	 //line3
		csae 3: x=0x98; break;	 //line4
    }
    pos=x+y;  
    write_com(pos);     //显示地址
}


/******************************************************
******************延时函数*****************************
******************************************************/
/*ms级延时函数*/
void delay_1ms(uint x) 
{ 
uint i,j; 
for(j=0;j<x;j++)  
	for(i=0;i<110;i++); 
}

