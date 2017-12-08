/*
S03:cancel 取消
S13:correct 更正
S23:change 修改
s33:cinfirm 确认

S00 到 S32
1 2 3
4 5 6
7 8 9 
  0
*/


#include<reg52.h>
#include<intrins.h>
#define uchar unsigned char
#define uint unsigned int
/*数码管*/
sbit dula=P2^6; //定义锁存器锁存端
sbit wela=P2^7;
/*液晶*/
sbit lcdrs=P3^5;		   //RS液晶数据命令选择端
sbit lcdrw=P3^6;		   //WR液晶读写选择端
sbit lcden=P3^4;		   //LCDEN液晶使能端
/*led*/
sbit led1=P1^0;	   //密码输入错误提示灯
sbit led2=P1^1;
sbit led3=P1^2;
sbit led7=P1^6;	   //密码位数不足提示灯
sbit led8=P1^7;	   //密码位数超标提示灯
/*蜂鸣器*/
sbit beep=P2^3;	   //蜂鸣器

/*******************子函数*****************************
******************************************************/
/*液晶*/
void yj_display(uint num);//液晶显示函数
void yj_write_com (uchar com);//液晶命令写入
void yj_write_data(uchar date);//液晶数据写入
/*特殊按键*/
void aj_Cancel();	   //取消
void aj_Change();	   //修改
void aj_Correct();	   //更正

/*延时*/
void delayms(uchar xms);//ms级延时函数
/*键盘扫描*/
void matrixkeyscan();//键盘扫描函数             
/*初始化*/
void init();	  //初始化函数

/*数组*/
uchar code inputtable[15]={"Input  Password"};//开始输入
uchar code changetable[13]={"Input New Key"}; //修改密码
uchar code againtable[11]={"Input Again"};	  //重新输入密码
uchar code righttable[15]={"Password  Right"};//输入正确
uchar code errortable[11]={"    Error  "};	  //输入错误
uchar code cleantable[16]={"                "};//清除输入
/*密码寄存数组*/
uchar table1[6]; //储存初始密码
uchar table2[6]; //储存输入的密码

uchar wei,error,xiugai;//wei:记录输入的密码的位数;error:记录还可以错误的次数;xiugai:修改开关
bit cancel,correct,change,confirm;//cancel:取消键;correct:更正键;change:修改键;confirm:确认键


void main()
{
	uchar i,temp;	 //temp:记录密码是否输入正确

	init();  //初始化
	while(1)
	{
		matrixkeyscan();	//键盘扫描子程序
		if(wei==6&&confirm==1)			//判断位数是否为6位并且已经按下了确认按键
		{
			temp=0;					 //初始密码输入正确
			for(i=0;i<6;i++)
			{
				if(table2[i]!=table1[i])//如果密码输入不正确
				{
					temp=1;			    //标记temp为1
					break;				//退出该for循环
				}
			}
			if(temp==1)				  //如果密码不正确
			{
				error--;	//剩余错误次数减1
				if(error==2)	//密码错误1次
					led1=0;		//开启led1
				if(error==1)	//密码错误2次
				{
					led1=0;		//开启led1
					led2=0;		//开启led2
				}
				if(error==0)	//密码错误3次
				{
					led1=0;		//开启led1
					led2=0;		//开启led2
					led3=0;		//开启led3
					beep=0;		//开启蜂鸣器
					while(1);
				}
				/*显示错误界面2秒*/
				yj_write_com(0x01);		 //00000001B,显示清0，数据指针清0
				yj_write_com(0x80);
				for(i=0;i<11;i++)
				{
					yj_write_data(errortable[i]);
				}
				yj_write_data(3-error+'0');	//提示错误次数
				for(i=10;i>0;i--)
					delayms(1000);	  //延时10s
				/*恢复重新显示界面*/
				yj_write_com(0x01);	   //00000001B,显示清0，数据指针清0
				wei=0;	   //记录输入的密码位数
				confirm=0;//清楚确认键标志
				yj_write_com(0x80);
				for(i=0;i<15;i++)
				{
					yj_write_data(inputtable[i]);
				}
				yj_write_com(0x80+0x40); 
				yj_write_com(0x0f);//光标闪烁
			}
			if(temp==0)			//如果密码正确
			{
				yj_write_com(0x01);	   //00000001B,显示清0，数据指针清0
				yj_write_com(0x80);
				for(i=0;i<15;i++)
				{
					yj_write_data(righttable[i]);	//显示输入正确的提示信息
				}
				yj_write_com(0x80+0x40);
				for(i=0;i<6;i++)
				{
					yj_write_data(table1[i]+'0');	//显示密码
				}
				xiugai=1;	//标记密码可修改开关打开
				do
				{
					uchar temp,key;
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
								case 0x7b:	change=1;	break;//修改键
							}
							while(temp!=0xf0)
							{
								temp=P3;
								temp=temp&0xf0;
							}
						}
					}
				}while(change==0);
				aj_Change();
			}
		}
	}
}

/*初始化*/
void init()
{
	uchar i;
	
	dula=0;
	wela=0;			 //关闭两锁存器锁存端，防止操作液晶时数码管会出乱码
	lcden=0;		//关闭液晶使能端
	/*初始化各变量*/
	wei=0;	   //记录输入的密码位数
	error=3;   //记录还可以错误的次数
	xiugai=0;	//关闭可修改开关
	cancel=0;
	correct=0;
	change=0;
	confirm=0;

	for(i=0;i<6;i++)
	{
		table1[i]=0;
		table2[i]='f';
	}
	
	/*初始化1602液晶*/
	yj_write_com(0x38);		 //00111000B,设置为16*2显示，5*7点阵，8位数据接口
	yj_write_com(0x0c);		 //00001100B,设置开显示，不显示光标
	yj_write_com(0x06);		 //00000110B,写一个字符后地址指针加1
	yj_write_com(0x01);		 //00000001B,显示清0，数据指针清0
	/*显示初始界面*/
	yj_write_com(0x80);
	for(i=0;i<15;i++)
	{
		yj_write_data(inputtable[i]);
	}
	yj_write_com(0x80+0x40);
	yj_write_com(0x0f);//光标闪烁
}


/******************************************************
*******************液晶显示****************************
******************************************************/
void yj_display(uchar num)
{
	if(wei<=5) //密码位数小于6时
	{	
		wei++;	
		yj_write_com(0x80+0x40+wei-1); //在液晶屏的下一位显示"*"
		yj_write_data('*');
		table2[wei-1]=num;	//将输入的密码储存到table2数组中
		if(wei==6)
		{
			yj_write_com(0x80+0x40+5);
			yj_write_com(0x0c);//取消光标闪烁
		}
	}
	else
	{
		wei=6;
		led8=0;
		beep=0;
		delayms(500);		//led8亮0.5s,伴着蜂鸣器响0.5s表示位数已到上限
		beep=1;
		led8=1;
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
*******************特殊按键****************************
******************************************************/
/*取消按键*/
void aj_Cancel()
{
	uchar i;

	cancel=0;
	/*初始化各变量*/
	wei=0;
	/*清楚第二行数据*/
	yj_write_com(0x80+0x40);
	for(i=0;i<6;i++)
	{
		yj_write_data(cleantable[i]);
	}
	for(i=0;i<6;i++)
	{
		table2[i]='f';	  	//将输入密码中的储存的密码设为其他码
	}
	yj_write_com(0x80+0x40);
}

/*更正按键*/
void aj_Correct()
{
	uchar i;

	correct =0;
	wei -= 1;	
	table2[wei]='f';
	yj_write_com(0x80+0x40+wei);
	yj_write_data(' ');
	yj_write_com(0x80+0x40+wei);
	yj_write_com(0x0f);
} 

/*修改按键*/
void aj_Change()
{
	uchar i,k;	//k:重新输入是否正确标记位
	wei=0;	   //记录输入的密码位数
	error=3;   //记录还可以错误的次数
	xiugai=0;	//关闭可修改开关
	change=0;
	confirm=0;
	yj_write_com(0x01);		 //00000001B,显示清0，数据指针清0
	/*显示修改界面*/
	yj_write_com(0x80);
	for(i=0;i<13;i++)	   //显示修改界面下的第一行数据
	{
		yj_write_data(changetable[i]);
	}
	yj_write_com(0x80+0x40); //光标移到第二行第一位
	yj_write_com(0x0f);	 //光标闪烁
	
	do
	{
		matrixkeyscan();	//获取新密码
	}while(wei<6||confirm==0);

	for(i=0;i<6;i++)
	{
		table1[i]=table2[i]; //将因为yj_display函数中储存的输入密码复制到原始密码中
		table2[i]='f';		 //将输入密码中的储存的密码设为其他码
	}
	
	do
	{
		confirm=0;		 //取消确认位
		k=1;			 //重复输入成功标志位标记1
		yj_write_com(0x01);		 //00000001B,显示清0，数据指针清0
		yj_write_com(0x80);	
		for(i=0;i<11;i++)		//显示重复输入模式下的第一行数据
		{
			yj_write_data(againtable[i]);
		}
		yj_write_com(0x80+0x40);//第二行显示
		yj_write_com(0x0f);		//光标显示

		wei=0;		  //清零输入的密码位数
 	
		do
		{
			matrixkeyscan();	//获取新密码的验证输入密码
		}while(wei<6||confirm==0);

		for(i=0;i<6;i++)
		{
			if(table2[i]!=table1[i])
			{
				k=0;		//输入错误,重复输入成功标志位标记0,要求重新输入
				break;
			}
		}
	}while(0==k);
	/*初始话显示*/
	wei=0;	   //记录输入的密码位数
	error=3;   //记录还可以错误的次数
	xiugai=0;	//关闭可修改开关
	cancel=0;
	correct=0;
	change=0;
	confirm=0;
	yj_write_com(0x01);		 //00000001B,显示清0，数据指针清0
	/*显示初始界面*/
	yj_write_com(0x80);
	for(i=0;i<15;i++)
	{
		yj_write_data(inputtable[i]);
	}
	yj_write_com(0x80+0x40);
	yj_write_com(0x0f);//光标闪烁
}


/******************************************************
*******************键盘扫描****************************
******************************************************/
/*矩阵键盘扫描*/
void matrixkeyscan()             
{
	uchar temp,key;
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
				case 0xee:	key=1;	break;	//数字1
				case 0xde:	key=2; 	break;	//数字2
				case 0xbe:	key=3;	break;	//数字3
				case 0x7e:	cancel=1;	break;//取消键
			}
			while(temp!=0xf0) //等待按键释放
			{
				temp=P3;
				temp=temp&0xf0;
			}
			if(cancel==0)	 //没有按取消键
			{
				yj_display(key);//显示数字
			}
			else 
			{	
				if(wei>=1)		 //至少已经输入了一个数字
					aj_Cancel();	 //取消输入,退出到主界面
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
				case 0xed:	key=4;	break;	//数字4
				case 0xdd:	key=5;	break;	//数字5
				case 0xbd:	key=6;	break;	//数字6
				case 0x7d:	correct=1;	break;//更正键
			}
			while(temp!=0xf0)
			{
				temp=P3;
				temp=temp&0xf0;
			}
			if(correct==0)	  //没有更正键
			{
				yj_display(key);  //显示数字
			}
			else 
			{
				if(wei>=1)		  //至少已经输入了一个数字
					aj_Correct();	  //更正上一个输入的字符
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
				case 0xeb:	key=7;	break;	//数字7
				case 0xdb:	key=8;	break;	//数字8
				case 0xbb:	key=9;	break;	//数字9
				case 0x7b:	change=1;	break;//修改键
			}
			while(temp!=0xf0)
			{
				temp=P3;
				temp=temp&0xf0;
			}
			if(change==0)	 //没有按修改键
			{
				yj_display(key);  //显示数字
			}
			else 
			{
				if(xiugai==1)	//如果可修改开关打开
					aj_Change();	//修改密码;
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
				case 0xd7:	key=0;	break;	//数字0
				case 0x77:	confirm=1;	break;//确认键	
			}
			while(temp!=0xf0)
			{
				temp=P3;
				temp=temp&0xf0;
			}	
			if(confirm==0)	//没有按确定键
			{
				yj_display(key);  //显示数字
			}
			else 
			{
				if(wei==6);
				else
				{	
					led7=0;
					delayms(500);	 //led7亮0.5s提示未输入满6位数据
					led7=1;
				}
			}
		}
	}
}


/******************************************************
*******************延时函数****************************
******************************************************/
/*ms级延时*/
void delayms(uint xms)   
{
 	uint i,j;
 	for(i=xms;i>0;i--)
		for(j=114;j>0;j--);
}









