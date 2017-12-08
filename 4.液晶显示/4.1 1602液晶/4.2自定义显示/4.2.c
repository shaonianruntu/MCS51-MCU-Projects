#include <reg52.h>
#define uchar unsigned char
#define uint unsigned int
uchar code table1[]={0x03,0x07,0x0f,0x1f,0x1f,0x1f,0x1f,0x1f,
               		 0x18,0x1e,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
                	 0x03,0x0f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
               		 0x18,0x1c,0x1e,0x1f,0x1f,0x1f,0x1f,0x1f,
               		 0x0f,0x07,0x03,0x01,0x00,0x00,0x00,0x00,
               		 0x1f,0x1f,0x1f,0x1f,0x1f,0x0f,0x07,0x01,
               		 0x1f,0x1f,0x1f,0x1f,0x1f,0x1e,0x1c,0x10,
               		 0x1e,0x1c,0x18,0x10,0x00,0x00,0x00,0x00};//心图案
uchar code table[]={0x00,0x07,0x04,0x04,0x04,0x14,0x0c,0x04};//√
uchar code hang1[16]={" HDU DIANZI     "};
uchar code hang2[16]={" FANGNAN        "};


/**************接口定义************************/
/*数码管*/
sbit dula=P2^6;
sbit wela=P2^7;
/*液晶*/
sbit lcden=P3^4;//LCDEN液晶使能端
sbit lcdrs=P3^5;//RS液晶数据命令选择端
sbit lcdrw=P3^6;//WR液晶读写选择端 

/**************子函数定义*****************************/
/*液晶*/
void yj_write_com(uchar com);//写入指令函数
void yj_write_data(uchar date);//写入数据函数
void init(void);//初始化函数
void delayms(uint xms);//ms级延时函数



/******************************************************
*******************主函数******************************
******************************************************/
void main()
{
	uchar i,j,k,tmp;  //tmp:字码的行
	init();
	tmp=0x40;//设置CGRAM地址的格式字
	k=0;
	for(j=0;j<8;j++)
    {
    	for(i=0;i<8;i++)			 //第一个字码
        {
        	yj_write_com(tmp+i); 	 //设置自定义字符的 CGRAM 地址 
         	yj_write_data(table1[k]);//向CGRAM写入自定义字符表的数据
         	k++;					 //k表示table数组中的第几个数
        }
      	tmp=tmp+8;	//自定义的5X8点阵的字符，需要8个字节来保存（高三位不用）。因此就有上面的对应关系了。
    }
	/*显示英文字母*/
	yj_write_com(0x80);	//第一行
	for(i=0;i<16;i++)
	{
		yj_write_data(hang1[i]);	
	}
	yj_write_com(0x80+0x40);//第二行
	for(i=0;i<16;i++)
	{
		yj_write_data(hang2[i]);	
	}
	/*显示爱心*/
    for(i=0;i<4;i++)//在第一行第12列位置显示心图案的上半部
    {
    	yj_write_com(0x80+12+i);
		yj_write_data(i);   	
    }
	for(i=4;i<8;i++)//在第二行第12列位置显示心图案的下半部
    {
    	yj_write_com(0x80+0x40+12+i-4);
		yj_write_data(i);   	
    }
  	
    while (1);
}


/*初始化*/
void init()
{
	dula=0;		 //关闭段选
	wela=0;		 //关闭位选
	lcden=0;	 //关闭液晶使能端

	/*初始化1602液晶*/
	yj_write_com(0x38);		 //00111000B,设置为16*2显示，5*7点阵，8位数据接口
	yj_write_com(0x0c);		 //00001100B,设置开显示，不显示光标
	yj_write_com(0x06);		 //00000110B,写一个字符后地址指针加1
	yj_write_com(0x01);		 //00000001B,显示清0，数据指针清0	
}


/******************************************************
*******************液晶显示****************************
******************************************************/
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


/******************************************************
*******************延时函数****************************
******************************************************/
/*ms级延时函数*/
void delayms(uint xms)
{
	uint i,j;
	for(i=xms;i>0;i--)
		for(j=114;j>0;j--);
}