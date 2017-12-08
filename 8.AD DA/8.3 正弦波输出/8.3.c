/******************************************************************** 
* 文件名  ： PCF8591T做DA实验.c 
* 描述    :  我们单片机去控制PCF8591T去实现DA功能。 
             这个代码也要在示波器上才能观测到，DA输出一个正弦波。 
* 可以用示波器测到正弦波 
***********************************************************************/  
  
#include<reg52.h>  
#include <I2C.H>  
  
#define  PCF8591 0x90    //PCF8591 地址  
  
// 此表为 LED 的字模            // 0    1    2    3    4    5    6   7    8    9    A    b    c    d    E    -    L   P    U    Hidden  _ (20)  
unsigned char Disp_Tab[] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x88,0x83,0xC6,0xA1,0x86,0xbf,0xc7,0x8c,0xc1, 0xff,  0xf7 };   
  
unsigned char AD_CHANNEL;  
unsigned long xdata  LedOut[8];  
unsigned int  idata D[32];  
// 128点正弦波样本表   
unsigned char code auc_SinParam[128] = {   
64,67,70,73,76,79,82,85,88,91,94,96,99,102,104,106,109,111,113,115,117,118,120,121,   
123,124,125,126,126,127,127,127,127,127,127,127,126,126,125,124,123,121,120,118,   
117,115,113,111,109,106,104,102,99,96,94,91,88,85,82,79,76,73,70,67,64,60,57,54,51,48,   
45,42,39,36,33,31,28,25,23,21,18,16,14,12,10,9,7,6,4,3,2,1,1,0,0,0,0,0,0,0,1,1,2,3,4,6,   
7,9,10,12,14,16,18,21,23,25,28,31,33,36,39,42,45,48,51,54,57,60};          
  
/******************************************************************* 
DAC 变换, 转化函数                
*******************************************************************/  
bit DACconversion(unsigned char sla,unsigned char c,  unsigned char Val)  
{  
   Start_I2c();              //启动总线  
   SendByte(sla);            //发送器件地址  
   if(ack==0)return(0);  
   SendByte(c);              //发送控制字节  
   if(ack==0)return(0);  
   SendByte(Val);            //发送DAC的数值    
   if(ack==0)return(0);  
   Stop_I2c();               //结束总线  
   return(1);  
}  
  
/******************************************************************* 
ADC发送字节[命令]数据函数                
*******************************************************************/  
bit ISendByte(unsigned char sla,unsigned char c)  
{  
   Start_I2c();              //启动总线  
   SendByte(sla);            //发送器件地址  
   if(ack==0)return(0);  
   SendByte(c);              //发送数据  
   if(ack==0)return(0);  
   Stop_I2c();               //结束总线  
   return(1);  
}  
  
/******************************************************************* 
ADC读字节数据函数                
*******************************************************************/  
unsigned char IRcvByte(unsigned char sla)  
{  unsigned char c;  
  
   Start_I2c();          //启动总线  
   SendByte(sla+1);      //发送器件地址  
   if(ack==0)return(0);  
   c=RcvByte();          //读取数据0  
  
   Ack_I2c(1);           //发送非就答位  
   Stop_I2c();           //结束总线  
   return(c);  
}  
/******************************************************************** 
* 名称 : Delay_1ms() 
* 功能 : 延时子程序，延时时间为 1ms * x 
* 输入 : x (延时一毫秒的个数) 
* 输出 : 无 
***********************************************************************/  
void Delay_1ms(unsigned char i)//1ms延时  
{  
    unsigned char x,j;  
    for(j=0;j<i;j++)  
    for(x=0;x<=148;x++);   
}  
//******************************************************************/  
main()  
{   
unsigned char i=0;  
 while(1)  
 {/********以下AD-DA处理*************/    
   switch(AD_CHANNEL)  
   {  
     case 0: ISendByte(PCF8591,0x41);  
             D[0]=IRcvByte(PCF8591)*2;  //ADC0 模数转换1  
             break;    
   
     case 1: ISendByte(PCF8591,0x42);  
             D[1]=IRcvByte(PCF8591)*2;  //ADC1  模数转换2  
             break;    
  
     case 2: ISendByte(PCF8591,0x43);  
             D[2]=IRcvByte(PCF8591)*2;  //ADC2  模数转换3  
             break;    
  
     case 3: ISendByte(PCF8591,0x40);  
             D[3]=IRcvByte(PCF8591)*2;  //ADC3   模数转换4  
             break;    
  
     case 4: DACconversion(PCF8591,0x40, D[4]/4); //DAC   数模转换  
             break;  
   }  
  
   if(++AD_CHANNEL>4) AD_CHANNEL=0;  
  
 /********以下将AD的值送到LED数码管显示*************/  
        D[4] = auc_SinParam[i++]*2;  
        Delay_1ms(5);  
        if(i==128) i=0;  
    }    
}  
 

view plaincopy to clipboardprint?
/*************************此部分为I2C总线的驱动程序*************************************/  
  
#include<reg52.h>  
#include <intrins.h>  
#include <I2C.H>  
  
#define  NOP()   _nop_()   /* 定义空指令 */  
#define  _Nop()  _nop_()   /*定义空指令*/  
  
sbit     SCL=P1^5;       //I2C  时钟   
sbit     SDA=P3^6;       //I2C  数据   
bit ack;                 /*应答标志位*/  
     
  
/******************************************************************* 
                     起动总线函数                
函数原型: void  Start_I2c();   
功能:     启动I2C总线,即发送I2C起始条件.   
********************************************************************/  
void Start_I2c()  
{  
  SDA=1;         /*发送起始条件的数据信号*/  
  _Nop();  
  SCL=1;  
  _Nop();        /*起始条件建立时间大于4.7us,延时*/  
  _Nop();  
  _Nop();  
  _Nop();  
  _Nop();      
  SDA=0;         /*发送起始信号*/  
  _Nop();        /* 起始条件锁定时间大于4μs*/  
  _Nop();  
  _Nop();  
  _Nop();  
  _Nop();         
  SCL=0;       /*钳住I2C总线，准备发送或接收数据 */  
  _Nop();  
  _Nop();  
}  
  
/******************************************************************* 
                      结束总线函数                
函数原型: void  Stop_I2c();   
功能:     结束I2C总线,即发送I2C结束条件.   
********************************************************************/  
void Stop_I2c()  
{  
  SDA=0;      /*发送结束条件的数据信号*/  
  _Nop();       /*发送结束条件的时钟信号*/  
  SCL=1;      /*结束条件建立时间大于4μs*/  
  _Nop();  
  _Nop();  
  _Nop();  
  _Nop();  
  _Nop();  
  SDA=1;      /*发送I2C总线结束信号*/  
  _Nop();  
  _Nop();  
  _Nop();  
  _Nop();  
}  
  
/******************************************************************* 
                 字节数据发送函数                
函数原型: void  SendByte(UCHAR c); 
功能:     将数据c发送出去,可以是地址,也可以是数据,发完后等待应答,并对 
          此状态位进行操作.(不应答或非应答都使ack=0)      
           发送数据正常，ack=1; ack=0表示被控器无应答或损坏。 
********************************************************************/  
void  SendByte(unsigned char  c)  
{  
 unsigned char  BitCnt;  
   
 for(BitCnt=0;BitCnt<8;BitCnt++)  /*要传送的数据长度为8位*/  
    {  
     if((c<<BitCnt)&0x80)SDA=1;   /*判断发送位*/  
       else  SDA=0;                  
     _Nop();  
     SCL=1;               /*置时钟线为高，通知被控器开始接收数据位*/  
      _Nop();   
      _Nop();             /*保证时钟高电平周期大于4μs*/  
      _Nop();  
      _Nop();  
      _Nop();           
     SCL=0;   
    }  
      
    _Nop();  
    _Nop();  
    SDA=1;                /*8位发送完后释放数据线，准备接收应答位*/  
    _Nop();  
    _Nop();     
    SCL=1;  
    _Nop();  
    _Nop();  
    _Nop();  
    if(SDA==1)ack=0;       
       else ack=1;        /*判断是否接收到应答信号*/  
    SCL=0;  
    _Nop();  
    _Nop();  
}  
  
/******************************************************************* 
                 字节数据接收函数                
函数原型: UCHAR  RcvByte(); 
功能:        用来接收从器件传来的数据,并判断总线错误(不发应答信号)， 
          发完后请用应答函数应答从机。   
********************************************************************/      
unsigned char   RcvByte()  
{  
  unsigned char  retc;  
  unsigned char  BitCnt;  
    
  retc=0;   
  SDA=1;                     /*置数据线为输入方式*/  
  for(BitCnt=0;BitCnt<8;BitCnt++)  
      {  
        _Nop();             
        SCL=0;                  /*置时钟线为低，准备接收数据位*/  
        _Nop();  
        _Nop();                 /*时钟低电平周期大于4.7μs*/  
        _Nop();  
        _Nop();  
        _Nop();  
        SCL=1;                  /*置时钟线为高使数据线上数据有效*/  
        _Nop();  
        _Nop();  
        retc=retc<<1;  
        if(SDA==1)retc=retc+1;  /*读数据位,接收的数据位放入retc中 */  
        _Nop();  
        _Nop();   
      }  
  SCL=0;      
  _Nop();  
  _Nop();  
  return(retc);  
}  
  
/******************************************************************** 
                     应答子函数 
函数原型:  void Ack_I2c(bit a); 
功能:      主控器进行应答信号(可以是应答或非应答信号，由位参数a决定) 
********************************************************************/  
void Ack_I2c(bit a)  
{  
    
  if(a==0)SDA=0;              /*在此发出应答或非应答信号 */  
  else SDA=1;  
  _Nop();  
  _Nop();  
  _Nop();        
  SCL=1;  
  _Nop();  
  _Nop();                    /*时钟低电平周期大于4μs*/  
  _Nop();  
  _Nop();  
  _Nop();    
  SCL=0;                     /*清时钟线，钳住I2C总线以便继续接收*/  
  _Nop();  
  _Nop();      
}  
 

view plaincopy to clipboardprint?
extern bit ack;  
//起动总线函数  
extern void Start_I2c();  
//结束总线函数    
extern void Stop_I2c();  
//应答子函数  
extern void Ack_I2c(bit a);  
//字节数据发送函数  
extern void  SendByte(unsigned char  c);  
//有子地址发送多字节数据函数                 
extern bit ISendStr(unsigned char sla,unsigned char suba,unsigned char *s,unsigned char no) ;  
//无子地址发送多字节数据函数     
extern bit ISendStrExt(unsigned char sla,unsigned char *s,unsigned char no);  
//无子地址读字节数据函数                 
extern unsigned char RcvByte();  
   
   
extern void HC595SendData(unsigned int SendVal);            
  
              
 

view plaincopy to clipboardprint?
/*************************此部分为74HC595的驱动程序使用SPI总线连接*************************************/  
  
#include<reg52.h>  
#include <intrins.h>  
  
#define  NOP()   _nop_()    /* 定义空指令 */  
#define  _Nop()  _nop_()    /*定义空指令*/  
  
//SPI IO  
sbit    MOSIO =P1^5;  
sbit    R_CLK =P1^6;  
sbit    S_CLK =P1^7;  
sbit    OE    =P3^6;  
  
void HC595SendData(unsigned int SendVal);  
  
  
/********************************************************************************************************* 
** 函数名称: HC595SendData 
** 功能描述: 向SPI总线发送数据 
** 管脚描述：请参考相关的芯片资料(学习光盘中以配) 
*********************************************************************************************************/  
void HC595SendData(unsigned int SendVal)  
{    
  unsigned char i;  
          
  for(i=0;i<16;i++)   
   {  
    if((SendVal<<i)&0x8000) MOSIO=1; // set dataline high  
    else MOSIO=0;  
   
    S_CLK=0;  
    NOP();  
    NOP();  
    S_CLK=1;      
   }  
     
      
  R_CLK=0; //set dataline low  
  NOP();  
  NOP();  
  R_CLK=1; //片选  
  OE=0;  
  
}  
 