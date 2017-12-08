#include<reg52.h>
#include<intrins.h>
#include<stdio.h>
#include<math.h>
#define uchar unsigned char
#define uint unsigned int

uchar code dutable[]={		//共阴极数码管段选码
//0   1    2	3 
0x3f,0x06,0x5b,0x4f,
//4   5	   6 	7
0x66,0x6d,0x7d,0x07,
//8   9	   .   关显示
0x7f,0x6f,0x80,0x00};
 
/*led*/
sbit led1=P1^0;
/*独立按键*/
sbit s1=P3^0;
sbit s2=P3^1;
sbit s3=P3^2;
sbit s4=P3^3;
/*数码管*/
sbit dula=P2^6;	 //段选
sbit wela=P2^7;	 //位选
/*液晶*/
sbit lcden=P3^4;//LCDEN液晶使能端
sbit lcdrs=P3^5;//RS液晶数据命令选择端
sbit lcdrw=P3^6;//WR液晶读写选择端
/*蜂鸣器*/
sbit beep=P2^3;

/*******************子函数*****************************
******************************************************/
/*液晶*/
void yj_fh_display(uchar a);//液晶符号显示
void yj_write_com (uchar com);//液晶命令写入
void yj_write_data(uchar date);//液晶数据写入
/*键盘*/
void matrixkeyscan();//矩阵按键扫描
void keyscan();//独立键盘扫描
/*初始化*/
void init();//初始化函数
/*延时*/
void delayms(uint xms);//ms级延时

void calculate();//计算


uchar code table[]={0x00,0x07,0x04,0x04,0x04,0x14,0x0c,0x04};   //√√

double su1,su2,su3;
unsigned long shu1,shu2,shu3; //shu1,shu2储存两个数字,有液晶屏长度限制,都定义不能大于6位数字,shu3为结果
uchar chang1,chang2,chang3;//chang1,chang2用来记录两个数字的长度,chang3记录结果总长
uchar digit1,digit2,digit3;//digit1,digit2记录shu1,shu2的整数位长度,digit3记录结果的整数长
	
uchar mark,fuhao=0; //mark:是否按下符号位的标记,按下了显示1,没有显示0 ;fuhao:记录输入的符号对应的代码;
bit point,equal,shift,fu;//equal:等于按键,按下为1;shift:是否按下扩展键;fu:记录减法得到的数是否为负数

/******************************************************
*******************主函数******************************
******************************************************/
void main()
{
	init();
	while(1)
	{
		
		matrixkeyscan();//矩阵键盘扫描
		keyscan();		//独立键盘扫描
	}
}

/*初始化函数*/
void init()
{
	dula=0;
	wela=0;			 //关闭两锁存器锁存端，防止操作液晶时数码管会出乱码
	lcden=0;		//关闭液晶使能端		
	/*初始化各变量*/
	shu1=0;
	shu2=0;
	shu3=0;
	chang1=0;
	chang2=0;
	chang3=0;
	digit1=0;
	digit2=0;
	digit3=0;
	mark=0;
	point=0;
	equal=0;
	shift=0;
	fu=0;

	/*初始化1602液晶*/
	yj_write_com(0x38);		 //00111000B,设置为16*2显示，5*7点阵，8位数据接口
	yj_write_com(0x0c);		 //00001100B,设置开显示，不显示光标
	yj_write_com(0x06);		 //00000110B,写一个字符后地址指针加1
	yj_write_com(0x01);		 //00000001B,显示清0，数据指针清0
	
	yj_write_com(0x80);
	yj_write_com(0x0f);//光标闪烁
}

/******************************************************
*******************键盘扫描****************************
******************************************************/
/*矩阵键盘扫描函数*/
void matrixkeyscan()
{
	uchar temp,key,input=0;	//key:记录输入的数字;input:标记是否输入了数字

	P3=0xfe;
	temp=P3;
	temp=temp&0xf0;
	if(temp!=0xf0)
	{
		delayms(10);
		temp=P3;
		temp=temp&0xf0;
		if(temp!=0xf0)	//重复判断,确认按键按下
		{
			temp=P3;
			switch(temp)
			{
				case 0xee:	key=1;	input=1;	break;	//数字1
				case 0xde:	key=2; 	input=1;	break;	//数字2
				case 0xbe:	key=3;	input=1;	break;	//数字3
				case 0x7e:	if(chang1!=0&&mark==0)	//如果已经输入了数字1,且没有输入符号
							{
								if(0==shift) //没有按下扩展键
								{
									mark=1;		//标记符号位
									fuhao=11;	//加号
								}
								if(1==shift)
								{	
									mark=1;
									fuhao=15;   //自然对数
								}
								break;
							}
			}
			while(temp!=0xf0) //等待按键释放
			{
				temp=P3;
				temp=temp&0xf0;
			}
			/*位数超标控制*/
			if(input==1&&0==mark&&chang1>=6)	//如果没有按符号位取消数1的继续输入,且数1的长度小于7
			{
				yj_write_com(0x80+chang1); 
				chang1=6; 
			}
			if(input==1&&2==mark&&chang2>=6)
			{
				if(16==fuhao)
					yj_write_com(0x80+chang1+3+chang2);
				else yj_write_com(0x80+chang1+1+chang2);  
				chang2=6;
			}
			/*输入显示*/
			if(1==input&&0==mark&&chang1<6)	//如果没有按符号位取消数1的继续输入,且数1的长度小于7
			{
				yj_write_com(0x80+chang1);  
				yj_write_data(key+'0');  //液晶显示数字
				shu1=shu1*10+key;	  //数1进位
				chang1++;			  //数1的位数增加
			}
			if(1==input&&2==mark&&chang2<6)
			{
				if(16==fuhao)
					yj_write_com(0x80+chang1+3+chang2);
				else yj_write_com(0x80+chang1+1+chang2);   
				yj_write_data(key+'0');  //液晶显示数字
				shu2=shu2*10+key;
				chang2++;
			}
			if(1==mark&&0==chang2)	 //显示符号
			{
				yj_fh_display(fuhao);
				mark=2;				 //标记符号位已输入完毕
			}
		}
	}
	
	P3=0xfd;
	temp=P3;
	temp=temp&0xf0;
	if(temp!=0xf0)
	{
		delayms(10);
		temp=P3;
		temp=temp&0xf0;	 //重复判断,确认按键按下
		if(temp!=0xf0)
		{
			temp=P3;
			switch(temp)
			{
				case 0xed:	key=4;	input=1;	break;	//数字4
				case 0xdd:	key=5;	input=1;	break;	//数字5
				case 0xbd:	key=6;	input=1;	break;	//数字6
				case 0x7d:	if(chang1!=0&&mark==0)	//如果已经输入了数字1,且没有输入符号
							{
								if(0==shift) //没有按下扩展键
								{
									mark=1;		//标记符号位
									fuhao=12;	//减号
								}
								if(1==shift)
								{	
									mark=1;
									fuhao=16;   //一般对数
								}
								break;
							}
			}
			while(temp!=0xf0) //等待按键释放
			{
				temp=P3;
				temp=temp&0xf0;
			}
			/*位数超标控制*/
			if(input==1&&0==mark&&chang1>=6)	//如果没有按符号位取消数1的继续输入,且数1的长度小于7
			{
				yj_write_com(0x80+chang1); 
				chang1=6; 
			}
			if(input==1&&2==mark&&chang2>=6)
			{
				if(16==fuhao)
					yj_write_com(0x80+chang1+3+chang2);
				else yj_write_com(0x80+chang1+1+chang2);  
				chang2=6;
			}
			/*输入显示*/
			if(1==input&&0==mark&&chang1<6)	//如果没有按符号位取消数1的继续输入,且数1的长度小于7
			{
				yj_write_com(0x80+chang1);  
				yj_write_data(key+'0');  //液晶显示数字
				shu1=shu1*10+key;	  //数1进位
				chang1++;			  //数1的位数增加
			}
			if(1==input&&2==mark&&chang2<6)
			{
				if(16==fuhao)
					yj_write_com(0x80+chang1+3+chang2);
				else yj_write_com(0x80+chang1+1+chang2);  
				yj_write_data(key+'0');  //液晶显示数字
				shu2=shu2*10+key;
				chang2++;
			}
			if(1==mark&&0==chang2)	 //显示符号
			{
				yj_fh_display(fuhao);
				mark=2;				 //标记符号位已输入完毕
			}
		}
	}

	P3=0xfb;
	temp=P3;
	temp=temp&0xf0;
	if(temp!=0xf0)
	{
		delayms(10);
		temp=P3;
		temp=temp&0xf0;	 //重复判断,确认按键按下
		if(temp!=0xf0)
		{
			temp=P3;
			switch(temp)
			{
				case 0xeb:	key=7;	input=1;	break;	//数字7
				case 0xdb:	key=8;	input=1;	break;	//数字8
				case 0xbb:	key=9;	input=1;	break;	//数字9
				case 0x7b:	if(chang1!=0&&mark==0)	//如果已经输入了数字1,且没有输入符号
							{
								if(0==shift) //没有按下扩展键
								{
									mark=1;		//标记符号位
									fuhao=13;	//乘号
								}
								if(1==shift)
								{	
									mark=1;
									fuhao=17;   //次方
								}
								break;
							}
			}
			while(temp!=0xf0) //等待按键释放
			{
				temp=P3;
				temp=temp&0xf0;
			}
			/*位数超标控制*/
			if(input==1&&0==mark&&chang1>=6)	//如果没有按符号位取消数1的继续输入,且数1的长度小于7
			{
				yj_write_com(0x80+chang1); 
				chang1=6; 
			}
			if(input==1&&2==mark&&chang2>=6)
			{
				if(16==fuhao)
					yj_write_com(0x80+chang1+3+chang2);
				else yj_write_com(0x80+chang1+1+chang2);  
				chang2=6;
			}
			/*输入显示*/
			if(input==1&&0==mark&&chang1<6)	//如果没有按符号位取消数1的继续输入,且数1的长度小于7
			{
				yj_write_com(0x80+chang1);  
				yj_write_data(key+'0');  //液晶显示数字
				shu1=shu1*10+key;	  //数1进位
				chang1++;			  //数1的位数增加
			}
			if(input==1&&2==mark&&chang2<6)
			{
				if(16==fuhao)
					yj_write_com(0x80+chang1+3+chang2);
				else yj_write_com(0x80+chang1+1+chang2);  
				yj_write_data(key+'0');  //液晶显示数字
				shu2=shu2*10+key;
				chang2++;
			}
			if(1==mark&&0==chang2)	 //显示符号
			{
				yj_fh_display(fuhao);
				mark=2;				 //标记符号位已输入完毕
			}
		}
	}

	P3=0xf7;
	temp=P3;
	temp=temp&0xf0;
	if(temp!=0xf0)
	{
		delayms(10);
		temp=P3;
		temp=temp&0xf0;	  //重复判断,确认按键按下
		if(temp!=0xf0)
		{
			temp=P3;
			switch(temp)
			{
			/*	case 0xe7:	point=1;		//小数点打开
							if(0==mark)
							{
								digit1=shu1; //数1的整数为数1目前的个数
							}
							if(2==mark)
							{
								digit2=shu2;	//数2的整数为数2目前的个数	
							}
							yj_write_data(0x80);
							break;	*/						
				case 0xd7:	key=0;	input=1;	break;	//数字0
				case 0xb7:	equal=1;	
							calculate();	   //进入计算程序
							break;
				case 0x77: 	if(chang1!=0&&mark==0)	//如果已经输入了数字1,且没有输入符号
							{
								if(0==shift) //没有按下扩展键
								{
									mark=1;		//标记符号位
									fuhao=14;	//除号
								}
								if(1==shift)
								{	
									mark=1;
									fuhao=18;   //开方
								}
								break;
							} 	
			}
			while(temp!=0xf0) //等待按键释放
			{
				temp=P3;
				temp=temp&0xf0;
			}
			/*位数超标控制*/
			if(input==1&&0==mark&&chang1>=6)	//如果没有按符号位取消数1的继续输入,且数1的长度小于7
			{
				yj_write_com(0x80+chang1); 
				chang1=6; 
			}
			if(input==1&&2==mark&&chang2>=6)
			{
				if(16==fuhao)
					yj_write_com(0x80+chang1+3+chang2);
				else yj_write_com(0x80+chang1+1+chang2);  
				chang2=6;
			}
			/*输入显示*/
			if(input==1&&0==mark&&chang1<6)	//如果没有按符号位取消数1的继续输入,且数1的长度小于7
			{
				yj_write_com(0x80+chang1);  
				yj_write_data(key+'0');  //液晶显示数字
				shu1=shu1*10+key;	  //数1进位
				chang1++;			  //数1的位数增加
			}
			if(input==1&&2==mark&&chang2<6)
			{
				if(16==fuhao)
					yj_write_com(0x80+chang1+3+chang2);
				else yj_write_com(0x80+chang1+1+chang2);  				yj_write_data(key+'0');  //液晶显示数字
				shu2=shu2*10+key;
				chang2++;
			}
			if(1==mark&&0==chang2)	 //显示符号
			{
				yj_fh_display(fuhao);
				mark=2;				 //标记符号位已输入完毕
			}
		}
	}
}

/*独立按键扫描程序*/
void keyscan()
{
	/*扩展键s1+s4*/
	if((0==s4)&&(0==s1))
	{
		delayms(100);	//延时500ms再判断s1是否仍按下
		if((0==s4)&&(0==s1))		//s1按键是否闭合
		{
			if(0==shift)
			{
				shift=1;
				led1=0;
				delayms(500);
			}
			else if(1==shift)
			{
				shift=0;
				led1=1;
				delayms(500);
			}
		}
	}

	/*退位键s1+s3*/
	if(0==s3&&0==s1)
	{
		delayms(100);
		if(0==s3&&0==s1)
		{
			if(0==mark&&chang1>0) //只输入了shu1
			{
				chang1--;
				yj_write_com(0x80+chang1);	 //光标移到前一位输入的数字的位置处
				yj_write_data(' ');			 //清楚该位的显示
				yj_write_com(0x80+chang1);
				shu1=shu1/10;				 //清楚刚才输入的末位数字	
			}
			else
			{
				if(2==mark&&chang2==0)	//输入了符号位却没有输入shu2
				{
					fuhao=0;		 //清楚符号位
					mark=0;			 //清楚符号标记位
					yj_write_com(0x80+chang1);	  //光标移到前一位输入的数字的位置处
					yj_write_data(' ');			  //清楚该位的显示
					yj_write_data(' ');			  //清楚该位的显示
					yj_write_data(' ');			  //清楚该位的显示
					yj_write_com(0x80+chang1);	  //光标移到前一位输入的数字的位置处	 
				}
				else
				{
					if(2==mark&&chang2!=0)	//输入了符号位却没有输入shu2
					{
						chang2--;
						if(16==fuhao)						 //光标移到前一位输入的数字的位置处
							yj_write_com(0x80+chang1+3+chang2);
						else yj_write_com(0x80+chang1+1+chang2); 
						yj_write_data(' ');			 		 //清楚该位的显示
						if(16==fuhao)						 //光标移到前一位输入的数字的位置处
							yj_write_com(0x80+chang1+3+chang2);
						else yj_write_com(0x80+chang1+1+chang2); 
						shu2=shu2/10;				 //清楚刚才输入的末位数字	
					}	
				}
			}
		}		
	}

	/*清屏键s2+s1*/
	if((0==s2)&&(0==s1))
	{
		delayms(100);
		if((0==s2)&&(0==s1))
		{
			/*初始化各变量*/
			shu1=0;
			shu2=0;
			shu3=0;
			chang1=0;
			chang2=0;
			chang3=0;
			digit1=0;
			digit2=0;
			digit3=0;
			mark=0;
			point=0;
			equal=0;
			shift=0;
			fu=0;		
		}

		yj_write_com(0x01);		 //00000001B,显示清0，数据指针清0
		yj_write_com(0x80);
		yj_write_com(0x0f);//光标闪烁
	}
}


/******************************************************
*******************计算程序****************************
******************************************************/
/*计算程序*/
void calculate()
{
	unsigned long a=1,b=0,c=0;
	uchar i;

	equal=0;	//清楚等号标记位

	if(chang1==0||(mark==2&&chang2==0))		   //没有输入任何数组或者已经显示了符号位,然而没有输入数2
	{
		if(fuhao==0)					  //没有输入任何数字
			yj_write_com(0x80);
		else
		{
			if(fuhao==16)				   //字符为log
				yj_write_com(0x80+chang1+3);
			else yj_write_com(0x80+chang1+1);	  //输入了字符
		}
	}
	else
	{
		if(chang2==0)					  //只输入了数字1
		{
			shu3=shu1;
			chang3=chang1;
		}
		else 
		{
			switch(fuhao)
			{
				case 11: shu3=shu1+shu2;
				/*加法*/ b=shu3;	  //将shu3的值暂存在b中
						 for(i=0;i<7;i++)
						 {
						 	if(b>0)			//如果shu3高位仍有数据则位数加1
								chang3++;
							b=b/10;
						 }
						 if(shu3==0)
							chang3=1;
						yj_write_com(0x80+0x40+15-chang3);
						yj_write_data('=');
						for(i=chang3;i>0;i--)
							a=a*10;
						for(i=0;i<chang3;i++)
						{
							a=a/10;
							b=shu3/a;	//做商,取第一位数字
							shu3=shu3%a;
							yj_write_data(b+'0');
						}	
						 break;

				case 12: if(shu1>shu2)	 
							shu3=shu1-shu2;
						 if(shu2>shu1)
						 {
						 	fu=1;
							shu3=shu2-shu1;
						 }
				/*减法*/ b=shu3;	  //将shu3的值暂存在b中
						 for(i=0;i<7;i++)
						 {
						 	if(b>0)			//如果shu3高位仍有数据则位数加1
								chang3++;
							b=b/10;
						 }
						 if(shu3==0)
							chang3=1;
						 if(0==fu)
						 {
						 	yj_write_com(0x80+0x40+15-chang3);
						 	yj_write_data('=');
						 }
						 else
						 {
						 	yj_write_com(0x80+0x40+15-chang3-1);
						 	yj_write_data('=');
							yj_write_data('-');
						 }
						 for(i=chang3;i>0;i--)
							a=a*10;
						 for(i=0;i<chang3;i++)
						 {
							a=a/10;
							b=shu3/a;	//做商,取第一位数字
							shu3=shu3%a;
							yj_write_data(b+'0');
						 }
						 fu=0; //清楚负数标记位	
						 break;

				case 13: shu3=shu1 * shu2;
				/*乘法*/ b=shu3;	  //将shu3的值暂存在b中
						 for(i=0;i<8;i++)
						 {
						 	if(b>0)			//如果shu3高位仍有数据则位数加1
								chang3++;
							b=b/10;
						 }
						 if(shu3==0)
							chang3=1;
					 	 yj_write_com(0x80+0x40+15-chang3);
		   				 yj_write_data('=');
				 		 for(i=chang3;i>0;i--)
							a=a*10;
						 for(i=0;i<chang3;i++)
					 	 {
							a=a/10;
							b=shu3/a;	//做商,取第一位数字
							shu3=shu3%a;
							yj_write_data(b+'0');
						 }	
						 break;

				case 14: shu3=shu1 * 1000 /shu2;
				/*除法*/ if(shu3%10>=5)		//4舍5入
						 	shu3=shu3+10;
						 shu3=shu3/10; 
						 		
						 b=shu3;
						 if(b<10)
						 {						  
						 	yj_write_com(0x80+0x40+15-4);
							yj_write_data('=');
							yj_write_data('0');
							yj_write_data('.');	  //显示小数点
							yj_write_data('0');
							yj_write_data(b+'0');
						 }
						 else
						 {
						 	if(b>=10&&b<100)
						 	{
						 		yj_write_com(0x80+0x40+15-4);
								yj_write_data('=');
								yj_write_data('0');
								yj_write_data('.');	  //显示小数点
								yj_write_data(b/10+'0');
								yj_write_data(b%10+'0');
							 }
							 else
							 {
							 	if(b>100)
								{
									for(i=0;i<8;i++)
						 			{
									 	if(b>0)			//如果shu3高位仍有数据则位数加1
											chang3++;
										b=b/10;
									 }
									 yj_write_com(0x80+0x40+15-chang3-1);
		   							 yj_write_data('=');
				 					 for(i=chang3-2;i>0;i--)
										a=a*10;
							 		 c=shu3/100;   //c寄存shu3的值
									 for(i=0;i<chang3-2;i++)
								 	 {
										a=a/10;
										b=c/a;	//做商,取第一位数字
										c=c%a;
										yj_write_data(b+'0');
						 			 }
									 yj_write_com(0x80+0x40+15-2);
									 yj_write_data('.');
									 yj_write_data((shu3%100)/10+'0');
									 yj_write_data(shu3%10+'0');			
								 } 
						 	 }
						 }
						 break;

				case 15: shu3=shu1 % shu2;
				/*取余*/ b=shu3;
						 for(i=0;i<8;i++)
						 {
						 	if(b>0)			//如果shu3高位仍有数据则位数加1
								chang3++;
							b=b/10;
						 }
						 if(shu3==0)
							chang3=1;
					 	 yj_write_com(0x80+0x40+15-chang3);
		   				 yj_write_data('=');
				 		 for(i=chang3;i>0;i--)
							a=a*10;
						 for(i=0;i<chang3;i++)
					 	 {
							a=a/10;
							b=shu3/a;	//做商,取第一位数字
							shu3=shu3%a;
							yj_write_data(b+'0');
						 }	
						 break;

				case 16: su3=(double)(log(shu2)) /(log(shu1));
				/*对数*/ shu3=su3*1000;
						 if(shu3%10>=5)		//4舍5入
						 	shu3=shu3+10;
						 shu3=shu3/10; 
						 		
						 b=shu3;
						 if(b<10)
						 {						  
						 	yj_write_com(0x80+0x40+15-4);
							yj_write_data('=');
							yj_write_data('0');
							yj_write_data('.');	  //显示小数点
							yj_write_data('0');
							yj_write_data(b+'0');
						 }
						 else
						 {
						 	if(b>=10&&b<100)
						 	{
						 		yj_write_com(0x80+0x40+15-4);
								yj_write_data('=');
								yj_write_data('0');
								yj_write_data('.');	  //显示小数点
								yj_write_data(b/10+'0');
								yj_write_data(b%10+'0');
							 }
							 else
							 {
							 	if(b>100)
								{
									for(i=0;i<8;i++)
						 			{
									 	if(b>0)			//如果shu3高位仍有数据则位数加1
											chang3++;
										b=b/10;
									 }
									 yj_write_com(0x80+0x40+15-chang3-1);
		   							 yj_write_data('=');
				 					 for(i=chang3-2;i>0;i--)
										a=a*10;
							 		 c=shu3/100;   //c寄存shu3的值
									 for(i=0;i<chang3-2;i++)
								 	 {
										a=a/10;
										b=c/a;	//做商,取第一位数字
										c=c%a;
										yj_write_data(b+'0');
						 			 }
									 yj_write_com(0x80+0x40+15-2);
									 yj_write_data('.');
									 yj_write_data((shu3%100)/10+'0');
									 yj_write_data(shu3%10+'0');			
								 } 
						 	 }
						 }
						 break;

				case 17: shu3=1;
				/*次方*/ for(i=0;i<shu2;i++)
						 	shu3 *= shu1;
						 b=shu3;	  //将shu3的值暂存在b中
						 for(i=0;i<9;i++)
						 {
						 	if(b>0)			//如果shu3高位仍有数据则位数加1
								chang3++;
							b=b/10;
						 }
						 if(shu3==0)
							chang3=1;
						 yj_write_com(0x80+0x40+15-chang3);
						 yj_write_data('=');
						 for(i=chang3;i>0;i--)
							a=a*10;
						 for(i=0;i<chang3;i++)
						 {
							a=a/10;
							b=shu3/a;	//做商,取第一位数字
							shu3=shu3%a;
							yj_write_data(b+'0');
						 }	
						 break;
							
				case 18: su1=shu1;
				/*开方*/ su2=shu2;
						 su3=pow(su1,1/su2);
			
						 shu3=su3*1000;
						 if(shu3%10>=5)		//4舍5入
						 	shu3=shu3+10;
						 shu3=shu3/10; 
						 		
						 b=shu3;
						 if(b<10)
						 {						  
						 	yj_write_com(0x80+0x40+15-4);
							yj_write_data('=');
							yj_write_data('0');
							yj_write_data('.');	  //显示小数点
							yj_write_data('0');
							yj_write_data(b+'0');
						 }
						 else
						 {
						 	if(b>=10&&b<100)
						 	{
						 		yj_write_com(0x80+0x40+15-4);
								yj_write_data('=');
								yj_write_data('0');
								yj_write_data('.');	  //显示小数点
								yj_write_data(b/10+'0');
								yj_write_data(b%10+'0');
							 }
							 else
							 {
							 	if(b>100)
								{
									for(i=0;i<8;i++)
						 			{
									 	if(b>0)			//如果shu3高位仍有数据则位数加1
											chang3++;
										b=b/10;
									 }
									 yj_write_com(0x80+0x40+15-chang3-1);
		   							 yj_write_data('=');
				 					 for(i=chang3-2;i>0;i--)
										a=a*10;
							 		 c=shu3/100;   //c寄存shu3的值
									 for(i=0;i<chang3-2;i++)
								 	 {
										a=a/10;
										b=c/a;	//做商,取第一位数字
										c=c%a;
										yj_write_data(b+'0');
						 			 }
									 yj_write_com(0x80+0x40+15-2);
									 yj_write_data('.');
									 yj_write_data((shu3%100)/10+'0');
									 yj_write_data(shu3%10+'0');			
								 } 
						 	 }
						 }
						 break;
						 		 
			}
		}
		
	}
}


/******************************************************
*******************液晶显示****************************
******************************************************/
/*液晶显示符号*/
void yj_fh_display(uchar a)
{
	uchar m;
	switch(a)
	{
		case 11: yj_write_com(0x80+chang1);
				 yj_write_data('+');	break;
		case 12: yj_write_com(0x80+chang1);
				 yj_write_data('-');	break;
		case 13: yj_write_com(0x80+chang1);
				 yj_write_data('*');	break;
		case 14: yj_write_com(0x80+chang1);
				 yj_write_data('/');	break;
		case 15: yj_write_com(0x80+chang1);
				 yj_write_data('%');	break;
		case 16: yj_write_com(0x80+chang1);
				 yj_write_data('l');
				 yj_write_data('o');
				 yj_write_data('g');
				 break;				 
		case 17: yj_write_com(0x80+chang1);
				 yj_write_data('^');	break;
		case 18: yj_write_com(0x40);
				 for(m=0;m<8;m++)
				 {
				 	yj_write_com(0x40+m);	 //设置自定义字符的 CGRAM 地址 
				 	yj_write_data(table[m]); //向CGRAM写入自定义字符表的数据
				 }
				 yj_write_com(0x80+chang1);
				 yj_write_data(0);
				 yj_write_com(0x80+chang1+1);
				 break;
	}
}


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