#include<reg52.h>

#define uchar unsigned char 
#define uint unsigned int
sbit dula=P2^6;		//段选
sbit wela=P2^7;		//位选
sbit lcden=P3^4;	//LCDEN液晶使能端
sbit lcdrs=P3^5;	//RS液晶数据命令选择端
sbit lcdwr=P3^6;	//WR液晶读写选择端
/*独立按键设置*/
sbit s1=P3^0;//功能键
sbit s2=P3^1;//增加键
sbit s3=P3^2;//减小键
sbit s4=P3^3;//闹钟查看键

sbit rd=P3^7;
sbit beep=P2^4;//蜂鸣器
sbit dscs=P1^4;
sbit dsas=P1^5;
sbit dsrw=P1^6;
sbit dsds=P1^7;
sbit dsirq=P3^3;
bit flag1,flag_ri;//定义两个位变量
uchar count,addr,flag,t0_num;//addr:光标选择位;
char miao,shi,fen,year,month,day,week,amiao,afen,ashi;
uchar code table[]=" 20  -  -      ";//液晶固定显示内容
uchar code table1[]="      :  :  ";
void write_ds(uchar,uchar);//函数申明
void set_alarm(uchar,uchar,uchar);void read_alarm();
uchar read_ds(uchar);void set_time();

void delayms(uint);			//毫秒级延时函数
void beeper();				//蜂鸣器报警函数
void write_com(uchar);		//液晶命令写入
void write_data(uchar);		//液晶数据写入
void write_sfm(uchar,char); //时分秒显示	 
void write_nyr(uchar,char); //年月日显示
void write_week(char weekday);//星期几显示

/*一般延时函数,ms级*/
void delayms(uint xms)
{
	uint x,y;
	for(x=xms;x>0;x--)
		for(y=114;y>0;y--);
}

/*蜂鸣器报警函数*/
void beeper() 
{
	beep=0;			 //开蜂鸣器
	delay(100);		 //0.1s延时
	beep=1;			 //关蜂鸣器
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

/*时分秒在第二行显示*/
void write_sfm(uchar add,char date)	 //1602液晶刷新时分秒函数4为时，7为分，10为秒
{
	char shi,ge;
	shi=date/10;
	ge=date%10;
	write_com(0x80+0x40+add);	  //第二行显示
	write_date(0x30+shi);
	write_date(0x30+ge);
}

/*年月日在第一行显示*/
void write_nyr(uchar add,char date)	 //1602液晶刷新年月日函数3为年，6为分，9为秒
{
	char shi,ge;
	shi=date/10;
	ge=date%10;
	write_com(0x80+add);		 //第一行显示
	write_date(0x30+shi);
	write_date(0x30+ge);
}

/*星期几在第一行显示*/
void write_week(char weekday)
{
	write_com(0x80+12);		   //星期几显示在第一行12~14位
	switch(weekday)
	{
		case 1:	write_date('M');delayms(2);
	/*星期一*/	write_date('O');delayms(2);
				write_date('N');
				break;
		case 2:	write_date('T');delayms(2);
	/*星期二*/	write_date('U');delayms(2);
				write_date('E');
				break;
		case 3:	write_date('W');delayms(2);
	/*星期三*/	write_date('E');delayms(2);
				write_date('D');
				break;
		case 4:	write_date('T');delayms(2);
	/*星期四*/	write_date('H');delayms(2);
				write_date('U');
				break;
		case 5:	write_date('F');delayms(2);
	/*星期五*/	write_date('R');delayms(2);
				write_date('I');
				break;
		case 6:	write_date('S');delayms(2);
	/*星期六*/	write_date('A');delayms(2);
				write_date('T');
				break;
		case 7:	write_date('S');delayms(2);
				write_date('U');delayms(2);
	/*星期日*/	write_date('N');
				break;
	}
}

/*独立键盘功能键扫描函数*/
void keyscan()
{
	if(flag_ri==1)
	{//这里用来取消闹钟报警，任一键取消报警
		if((s1==0)||(s2==0)||(s3==0)||(s4==0))
		{	
			delay(5);
			if((s1==0)||(s2==0)||(s3==0)||(s4==0))
			{
				while(!(s1&&s2&&s3&&s4));
                beeper();
				flag_ri=0;//清除报警标志
			}	 	
		}
	}	

	if(s1==0)//检测S1
	{
		delayms(5);		  			//延时防抖
		if(s1==0)					//重复判断,防抖
		{	
			addr++;					//光标进位
			beeper();				//蜂鸣器响作修改反馈
			while(!s1);				//等待按键恢复
			
			switch(addr)			//光标闪烁点定位
			{
				case 1: write_com(0x80+0x40+10);  //秒位(二排11位)
					    write_com(0x0f);		  //光标闪烁
						break;
				case 2: write_com(0x80+0x40+7);	  //分位(二排8位)
						break;
				case 3: write_com(0x80+0x40+4);	  //时位(二排5位)
						break;
				case 4: write_com(0x80+12);		  //星期位(一排13位)
						break;
				case 5: write_com(0x80+9);		  //日位(一排10位)
						break;
				case 6: write_com(0x80+6);		  //月位(一排7位)
						break;
				case 7: write_com(0x80+3);		  //年位(一排4位)
						break;
				case 8: addr=0;					  //光标选择位清零(取消选择)
						write_com(0x0c);		  //开显示,但取消光标闪烁
					
						write_ds(0,miao);
						write_ds(2,fen);
						write_ds(4,shi);
						write_ds(6,week);
						write_ds(7,day);
						write_ds(8,month);
						write_ds(9,year);
						break;
			}	
		}
	}
	if(addr!=0)//只有当S1按下后，才检测S2和S3
	{
		if(s2==0)
		{
			delay(1);
			if(s2==0)
			{
				while(!s2);beeper();
				switch(addr)
				{//根据功能键次数调节相应数值
					case 1:	miao++;
							if(miao==60)
								miao=0;
							write_sfm(10,miao);
							write_com(0x80+0x40+10); 
							break;
					case 2:	fen++;
							if(fen==60)
								fen=0;
							write_sfm(7,fen);
							write_com(0x80+0x40+7); 
							break;
					case 3:	shi++;
							if(shi==24)
								shi=0;
							write_sfm(4,shi);
							write_com(0x80+0x40+4); 
							break;
					case 4:	week++;
							if(week==8)
								week=1;
							write_week(week);
							write_com(0x80+12);
							break;
					case 5:	day++;
							if(day==32)
								day=1;
							write_nyr(9,day);
							write_com(0x80+9);
							break;
					case 6:	month++;
							if(month==13)
								month=1;
							write_nyr(6,month);
							write_com(0x80+6);
							break;
					case 7:	year++;
							if(year==100)
								year=0;
							write_nyr(3,year);
							write_com(0x80+3);
							break;
				}
			}
		}
		if(s3==0)
		{
			delay(1);
			if(s3==0)
			{
				while(!s3);beeper();
				switch(addr)
				{//根据功能键次数调节相应数值
					case 1:	miao--;
							if(miao==-1)
								miao=59;
							write_sfm(10,miao);
							write_com(0x80+0x40+10);
							break; 
					case 2:	fen--;
							if(fen==-1)
								fen=59;
							write_sfm(7,fen);
							write_com(0x80+0x40+7);
							break;
					case 3:	shi--;
							if(shi==-1)
								shi=23;
							write_sfm(4,shi);
							write_com(0x80+0x40+4);
							break;
					case 4:	week--;
							if(week==0)
								week=7;
							write_week(week);
							write_com(0x80+12);
							break;
					case 5:	day--;
							if(day==0)
								day=31;
							write_nyr(9,day);
							write_com(0x80+9);
							break;
					case 6:	month--;
							if(month==0)
								month=12;
							write_nyr(6,month);
							write_com(0x80+6);
							break;
					case 7:	year--;
							if(year==-1)
								year=99;
							write_nyr(3,year);
							write_com(0x80+3);
							break;
				}
			}
		}
	}
	if(s4==0)//检测S4
	{
		delay(5);
		if(s4==0)
		{	
			flag1=~flag1;
			while(!s4);beeper();
			if(flag1==0)
			{//退出闹钟设置时保存数值
				flag=0;
				write_com(0x80+0x40);
				write_date(' ');
				write_date(' ');
				write_com(0x0c);
				write_ds(1,miao);
				write_ds(3,fen);
				write_ds(5,shi);				
			}
			else
			{//进入闹钟设置
				read_alarm();//读取原始数据
				miao=amiao;//重新赋值用以按键调节
				fen=afen;
				shi=ashi;
				write_com(0x80+0x40);
				write_date('R');//显示标志
				write_date('i');
				write_com(0x80+0x40+3);
				write_sfm(4,ashi);//送液晶显示闹钟时间
				write_sfm(7,afen);
				write_sfm(10,amiao);
			}			
		}
	}
}

void write_ds(uchar add,uchar date)
{//写12C887函数
	dscs=0;
	dsas=1;
	dsds=1;
	dsrw=1;
	P0=add;//先写地址
	dsas=0;
	dsrw=0;
	P0=date;//再写数据
	dsrw=1;
	dsas=1;
	dscs=1;				
}

uchar read_ds(uchar add)
{//读12C887函数
 	uchar ds_date;
	dsas=1;
	dsds=1;
	dsrw=1;
	dscs=0;
	P0=add;//先写地址
	dsas=0;
	dsds=0;
	P0=0xff;
	ds_date=P0;//再读数据
	dsds=1;
	dsas=1;
	dscs=1;
	return ds_date;	
}

void read_alarm()
{//读取12C887闹钟值
	amiao=read_ds(1);
	afen=read_ds(3);
	ashi=read_ds(5);
}
void main()//主函数
{
	init();//调用初始化函数
	while(1)
	{
		keyscan();//按键扫描
		if(flag_ri==1)//当闹钟中断时进入这里
		{
			beeper();
			delay(100);
			beeper();
			delay(500);
		}	
		if(flag==0&&flag1==0)//正常工作时进入这里
		{
			keyscan();//按键扫描
			year=read_ds(9);//读取12C887数据
			month=read_ds(8);	
			day=read_ds(7);
			week=read_ds(6);
			shi=read_ds(4);
			fen=read_ds(2);
			miao=read_ds(0);	
			write_sfm(10,miao);//送液晶显示
			write_sfm(7,fen);
			write_sfm(4,shi);
			write_week(week);
			write_nyr(3,year);
			write_nyr(6,month);
			write_nyr(9,day);
		}
	}
}

void exter() interrupt 2//外部中断1服务程序
{	uchar c; //进入中断表示闹钟时间到，
	flag_ri=1; //设置标志位，用以大程序中报警提示
	c=read_ds(0x0c);//读取12C887的C寄存器表示响应了中断
}

void init()
{//初始化函数

	dula=0;			  //关闭数码管显示
	wela=0;
	lcden=0;		  //关闭液晶使能端

	uchar num;
	EA=1;//打开总中断
	EX1=1;//开外部中断1
	IT1=1;//设置负跳变沿触发中断
	flag1=0;//变量初始化
	t0_num=0;
	addr=0;
	week=1;

	rd=0;
/*以下几行在首次设置DS12C887时使用，以后不必再写入
	write_ds(0x0A,0x20);//打开振荡器
	write_ds(0x0B,0x26);//设置24小时模式，数据二进制格式
						//开启闹铃中断
	set_time();//设置上电默认时间
-----------------------------------------------------*/
	write_com(0x38);		 //00111000B,设置为16*2显示，5*7点阵，8位数据接口
	write_com(0x0c);		 //00001100B,设置开显示，不显示光标
	write_com(0x06);		 //00000110B,写一个字符后地址指针加1
	write_com(0x01);		 //00000001B,显示清0，数据指针清0
	

	write_com(0x80);
	for(num=0;num<15;num++)//写入液晶固定部分显示
		{
			write_date(table[num]);
			delay(1);
		}
	write_com(0x80+0x40);
	for(num=0;num<11;num++)
		{
			write_date(table1[num]);
			delay(1);
		}
}
