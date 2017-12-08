#include<reg52.h>
#define uint unsigned int 
#define uchar unsigned char
sbit led1=P1^0;
void delay(uint s);
void main()
{
	uint cycle=600,pwm=0;              //定义周期
	while(1)
	{
		led1=1;
		delay(60000);				   //暗状态延时，可以看见熄灭的过程
		for(pwm=1;pwm<cycle;pwm++)	   //呼吸灯由暗到明变化
		{
			led1=0;					   //呼吸灯以非常快的速度开-关-开-关，
			delay(pwm);				   //并逐渐加大开的时间比率，减小关的
			led1=1;					   //时间比率，从而时间由暗到明的变化。
			delay(cycle-pwm);
		}	
		led1=0;
		for(pwm=cycle-1;pwm>0;pwm--)   //呼吸灯由明到暗变化
		{
			led1=0;
			delay(pwm);
			led1=1;
			delay(cycle-pwm);
		}
	}
}	

/*延时程序*/
void delay(uint s)
{
	while(--s);
}