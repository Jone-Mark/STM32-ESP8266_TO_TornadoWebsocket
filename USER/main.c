/*
�Ƚϼ�һ��С���򣬻������ԱȽ��ȶ���
�������⣬û�м��쳣����
WEbsocket��http://47.106.209.211:8888/

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
char *ssid="iot 921";  //wifi����
char *passwd="iot123456";   //����

int main(void)
{		
	u8 TCP_DATA[]="";
	char id=0;         //id����
	u32 value=0;       //��Ӧֵ
	
	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);	 //���ڳ�ʼ��Ϊ115200
  usart3_init(115200);		//��ʼ������3 
 	LED_Init();			     //LED�˿ڳ�ʼ��
	KEY_Init();          //��ʼ���밴�����ӵ�Ӳ���ӿ�
	ESP8266_Init();      //��ʼ��ESP8266���
	
 	while(1)
	{
		id+=1;value+=2;
		if(id>9)id=0;
		
		sprintf(TCP_DATA,"GET /api?id=%d&value=%d HTTP/1.1\r\nHOST:47.106.209.211:8888\r\n",id,value);
//		printf(TCP_DATA);
		delay_ms(1000);          //ÿ���һ���������������һ������
		send_8266_cmd(TCP_DATA,"OK",200);//����TCP
		
		LED1=!LED1;//�̵���
	}	 
}
 
 
 
u8 send_8266_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
	u16 len;
	u16 t; 
	USART3_RX_STA=0;
	u3_printf("%s\r\n",cmd);	//��������
	if(ack&&waittime)		//��Ҫ�ȴ�Ӧ��
	{
		while(--waittime)	//�ȴ�����ʱ
		{
			delay_ms(10);
			if(USART3_RX_STA&0X8000)//���յ��ڴ���Ӧ����
			{
				
				len=USART3_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
				for(t=0;t<len;t++)
				{
					USART_SendData(USART1, USART3_RX_BUF[t]);//�򴮿�1��������
					while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
				}
				if(atk_8266_check_cmd(ack))
				{
					printf("ack:%s\r\n",(u8*)ack);
					break;//�õ���Ч���� 
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
	if(USART3_RX_STA&0X8000)		//���յ�һ��������
	{ 
		USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;//��ӽ�����
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
	
	while(send_8266_cmd("AT","OK",20))//���WIFIģ���Ƿ�����
	{
		LED0=!LED0;//�����
		delay_ms(800);
		printf("WIFIģ��δ��ȷ����\r\n");	
	} 
	LED0=1;
	printf("WIFIģ��������\r\n");	
	
	send_8266_cmd("AT+CIPMODE=3","OK",200);  //����wifiӦ��ģʽ
	send_8266_cmd("AT+RST","OK",1000);  //����
	sprintf(cwjap,"AT+CWJAP=\"%s\",\"%s\"",ssid,passwd);
	printf(cwjap);
	send_8266_cmd(cwjap,"OK",1000);  //��������·����
	LED1=0;

	send_8266_cmd("AT+CIPSTART=\"TCP\",\"47.106.209.211\",8888","OK",200);//����TCP
	send_8266_cmd("AT+CIPMODE=1","OK",200);  //����͸��ģʽ
	send_8266_cmd("AT+CIPSEND","OK",200);  //��������
}

