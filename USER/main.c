/*
比较简单一个小程序，基本测试比较稳定。
存在问题，没有加异常处理
WEbsocket：http://47.106.209.211:8888/

*/

#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"
#include "usart3.h"
#include "string.h" 

u8 send_8266_cmd(u8 *cmd,u8 *ack,u16 waittime);
u8* atk_8266_check_cmd(u8 *str);
void RST_8266(void);
void ESP8266_Init(void);
	
u8 cwjap[]="";
char *ssid="iot 921";  //wifi名称
char *passwd="iot123456";   //密码

int main(void)
{		
	u8 TCP_DATA[]="";
	char id=0;         //id数据
	u32 value=0;       //对应值
	
	delay_init();	    	 //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(115200);	 //串口初始化为115200
  usart3_init(115200);		//初始化串口3 
 	LED_Init();			     //LED端口初始化
	KEY_Init();          //初始化与按键连接的硬件接口
	ESP8266_Init();      //初始化ESP8266相关
	
 	while(1)
	{
		id+=1;value+=2;
		if(id>9)id=0;
		
		sprintf(TCP_DATA,"GET /api?id=%d&value=%d HTTP/1.1\r\nHOST:47.106.209.211:8888\r\n",id,value);
//		printf(TCP_DATA);
		delay_ms(1000);          //每间隔一秒钟向服务器传输一次数据
		send_8266_cmd(TCP_DATA,"OK",200);//连接TCP
		
		LED1=!LED1;//绿灯闪
	}	 
}
 
 
 
u8 send_8266_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
	u16 len;
	u16 t; 
	USART3_RX_STA=0;
	u3_printf("%s\r\n",cmd);	//发送命令
	if(ack&&waittime)		//需要等待应答
	{
		while(--waittime)	//等待倒计时
		{
			delay_ms(10);
			if(USART3_RX_STA&0X8000)//接收到期待的应答结果
			{
				
				len=USART3_RX_STA&0x3fff;//得到此次接收到的数据长度
				for(t=0;t<len;t++)
				{
					USART_SendData(USART1, USART3_RX_BUF[t]);//向串口1发送数据
					while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
				}
				if(atk_8266_check_cmd(ack))
				{
					printf("ack:%s\r\n",(u8*)ack);
					break;//得到有效数据 
				}
					USART3_RX_STA=0;
			} 
		}
		if(waittime==0)res=1; 
	}
	return res;
} 

u8* atk_8266_check_cmd(u8 *str)
{
	
	char *strx=0;
	if(USART3_RX_STA&0X8000)		//接收到一次数据了
	{ 
		USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;//添加结束符
		strx=strstr((const char*)USART3_RX_BUF,(const char*)str);
	} 
	return (u8*)strx;
}

void RST_8266(void)
{
	ESP8266_HIGH;	
	ESP8266_LOW;
	delay_ms(200);
  ESP8266_HIGH;	
}

void ESP8266_Init(void)
{
	RST_8266();
	
	while(send_8266_cmd("AT","OK",20))//检查WIFI模块是否在线
	{
		LED0=!LED0;//红灯闪
		delay_ms(800);
		printf("WIFI模块未正确连接\r\n");	
	} 
	LED0=1;
	printf("WIFI模块已连接\r\n");	
	
	send_8266_cmd("AT+CIPMODE=3","OK",200);  //设置wifi应用模式
	send_8266_cmd("AT+RST","OK",1000);  //重启
	sprintf(cwjap,"AT+CWJAP=\"%s\",\"%s\"",ssid,passwd);
	printf(cwjap);
	send_8266_cmd(cwjap,"OK",1000);  //连接无线路由器
	LED1=0;

	send_8266_cmd("AT+CIPSTART=\"TCP\",\"47.106.209.211\",8888","OK",200);//连接TCP
	send_8266_cmd("AT+CIPMODE=1","OK",200);  //进入透传模式
	send_8266_cmd("AT+CIPSEND","OK",200);  //发送命令
}

