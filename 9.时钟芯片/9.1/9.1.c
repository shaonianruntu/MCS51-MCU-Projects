#include<reg52.h>
#define uchar unsigned char
#define uint unsigned int

/*DS1302与at89s52引脚连接*/
//管脚定义
sbit SCLK=P3^5;           //DS1302时钟信号  7脚       
sbit DIO=P3^6;            //DS1302数据信号  6脚     
sbit CE=P3^7;  		  	  //DS1302片选		5脚
//位寻址寄存器定义              
sbit ACC0=ACC^0;		  //ACC累加器(ACC:8位寄存器)的D0位
sbit ACC7=ACC^7;		  //ACC累加器的D7位

sbit dula=P2^6;			  //段选
sbit wela=P2^7;			  //位选

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

uchar code wetable[]={		//数码管位选码
//第1位 2位	 3位   4位       
0xfe, 0xfd, 0xfb, 0xf7, 
//5位  6位	 7位   8位
0xef, 0xdf, 0xbf, 0x7f};


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

/*一般延时函数*/
void delayms(uchar xms)
{
   	uchar i,j;
   	for(i=xms;i>0;i--)
   		for(j=114;j>0;j--);
}


/*显示程序*/
/* 动态扫描条件（单个LED在1秒内）：  count >=50次   //点亮次数               */
/*                                   time  >=2ms    //持续时间               */
/* DS1302秒，分，时寄存器是BCD码形式：用16求商和余进行"高4位"和"低4位"分离 */
/****************************************************************************/           
void display(uchar *poi)
{  
   dula=1;
   P0=dutable[*poi % 16];	   //显示秒个位	
   dula=0;
   P0=0xff;					   //消影
   wela=1;
   P0=wetable[7];			   //第8个数码管
   wela=0;
   delayms(3);                 //延时
                    
   dula=1;
   P0=dutable[*poi /16];       //显示秒十位        
   dula=0;
   P0=0xff;					   //消影
   wela=1;
   P0=wetable[6];			   //第7个数码管
   wela=0;
   delayms(3);				   //延时
   poi++;					   //地址进一

   dula=1;
   P0=0x40;            		   //"-"分隔符
   dula=0;
   P0=0xff;					   
   wela=1;
   P0=wetable[5];			   //第6个数码管
   wela=0;
   delayms(3);				   

   dula=1;
   P0=dutable[*poi % 16];      //显示分个位         
   dula=0;
   P0=0xff;					   //消影
   wela=1;
   P0=wetable[4];			   //第5个数码管
   wela=0;
   delayms(3);

   dula=1;
   P0=dutable[*poi /16];       //显示分十位        
   dula=0;
   P0=0xff;
   wela=1;
   P0=wetable[3];			   //第4个数码管
   wela=0;
   delayms(3);
   poi++;

   dula=1;
   P0=0x40;            		   //"-"分隔符
   dula=0;
   P0=0xff;					   
   wela=1;
   P0=wetable[2];			   //第3个数码管
   wela=0;
   delayms(3);	

   dula=1;
   P0=dutable[*poi %16];       //显示时个位        
   dula=0;
   P0=0xff;
   wela=1;
   P0=wetable[1];			   //第2个数码管
   wela=0;
   delayms(3);
  
   dula=1;
   P0=dutable[*poi /16];       //显示时十位        
   dula=0;
   P0=0xff;
   wela=1;
   P0=wetable[0];			   //第1个数码管
   wela=0;
   delayms(3);
}



void main(void)
{
  	 uchar clock[3]={0x00,0x00,0x12};   //秒，分，时寄存器初始值
  	 uchar temp=0x80;          			//写寄存器的地址为0x80,偶数位
  	 uchar i;
   	 write_1302(0x8e,0x00);             //WP=0 写操作
  	 for(i=0;i<3;i++)					//写入时间初值
  	 {
   	 	write_1302(temp,clock[i]);		
  		temp+=2;
  	 }
  	 write_1302(0x8e,0x80);             //WP=1 写保护
  	 while(1)
  	 {  
   	 	display(clock);				    //数码管显示时间
		temp=0x81;                    	//读寄存器的地址为0x81,奇数位
		for(i=0;i<3;i++)
 	 	{
	    	clock[i]=read_1302(temp);	//读取时间
		 	temp+=2;
	  	}
    }
}

// ds1302_writeData(0x90,0x5c); //禁止充电,降低功耗,针对不可充电电池  
//ds1302_writeData(0x90, 0xa6);//开启充电,用一个二极管,用4k电阻 
