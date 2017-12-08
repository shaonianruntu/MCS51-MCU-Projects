#include<reg52.h>
#define uchar unsigned char
#define uint unsigned int  
#define ADD 0x02          //从机地址
#define BN 12
uchar code table[12]={"HDU FangNan"};		
uchar rable[12];			//code定义的变量是写入ROM的,不能够修改,这里不能加code

void init();				//串行口初始化
uchar master(uchar,uchar);	//主机通信
void error();				//发送复位信号

void main()
{
	init();				  //串行口初始化
	master(ADD,0x01);	  
	master(ADD,0x02);
	while(1);
}

/*串行口初始化*/
void init()
{
	TMOD=0x20;		//设置定时器1的工作模式2,8位自动重装
	TH1=253;		//9600比特率
	TL1=253;
	PCON=0x00;		//
	TR1=1;			//启动定时器1

	SCON=0xf8;		//SCON: SM0 SM1 SM2 REN TB8 RB8 TI  RI 
					//50h =  1   1   1   1   1   0  0   0 b
					//串行口为方式3
}

/*主机通信*/
uchar master(uchar addr,uchar command)
{
	uchar a,temp,i;
	while(1)
	{
		SBUF=addr;					  //发送呼叫地址
		while(!TI);					  //等待发送完毕
		TI=0;
		while(!RI);					  //等待从机回答
		RI=0;
		if(SBUF!=addr)				  //判断应答地址是否相符
			error();				  //若地址错,发复位信号
		else				   		  //地址相符
		{
		 	TB8=0;					  //清地址标志
			SBUF=command;	 		  //发命令
			while(!TI);				  //等待主机发送完毕
				TI=0;
			while(!RI);		 		  //等到从机应答
				RI=0;
			a=SBUF;			  		  //接收状态
			if((a&0x80)==0x80)		  //若命令未被接受,发送复位信号
			{
				TB8=1;
				error();
			}
			else 
			{
				if(command==0x01)		//是发送命令
				{
					if((a&0x01)==0x01)	//从机准备好接收
					{
						do
						{
							temp=0;		//清校验和
							for(i=0;i<BN;i++)
							{
								SBUF=table[i];	 //发送一数据
								temp+=table[i];
								while(!TI);
									TI=0;
							}
							SBUF=temp;			 //发送校验和
							while(!TI);
								TI=0;
							while(!RI);
								RI=0;
						}while(SBUF!=0);		//接收不正确,等待重新发送(接收正确时会送正确信号0x00,命令从机复位)
						TB8=1;				//置地址标志,作为结束标志
						return 0;
					}
				}
				else 
				{
					if((a&0x02)==0x02)	   //接收命令,从机准备好发送
					{
						while(1)
						{
							temp=0;			//清校验和
							for(i=0;i<BN;i++)
							{
								while(!RI);	 //等待接收完毕
									RI=0;
								rable[i]=SBUF;	   //接收一位数据
								temp+=rable[i];
							}
							while(!RI);
								RI=0;
							if(SBUF==temp)		  //校验和校对
							{
								SBUF=0x00;		  //校验和相同发送00
								while(!TI);
									TI=0;
								break;
							}
							else
							{
								SBUF=0xff;		  //校验和不同发ff,重新接收
								while(!TI);
									TI=0;
							}
						}
						TB8=1;			//置地址标志,作为结束标志
						return 0;	
					}
				}
			}
		}
	}
}

/*发送复位信号*/
void error()
{
	SBUF=0xff;				  
	while(!TI);
	TI=0;
}