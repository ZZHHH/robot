#include "sys.h"
#include "usart.h"

//加入以下代码,支持printf函数,而不需要选择use MicroLIB
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

/*使用microLib的方法*/
 /* 
int fputc(int ch, FILE *f)
{
	USART_SendData(USART1, (uint8_t) ch);

	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {}	
   
    return ch;
}
int GetKey (void)  { 

    while (!(USART1->SR & USART_FLAG_RXNE));

    return ((int)(USART1->DR & 0x1FF));
}
*/
 
u8 USART_RX_BUF[64];     //接收缓冲,最大64个字节,存储的是ASCII码！一个数字或字母或字符为一字节
//接收状态
//bit7，接收完成标志
//bit6，接收到0x0d
//bit5~0，接收到的有效字节数目
u8 USART_RX_STA=0;       //接收状态标记

void uart_init(u32 bound)
{
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);
     
	//USART1_TX   PA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
   
    //USART1_RX	  PA.10
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);  

   //Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;

	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);
  
   //USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;					//bps
	USART_InitStructure.USART_WordLength = USART_WordLength_8b; //传输或接收数据位数
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		//停止位数目
	USART_InitStructure.USART_Parity = USART_Parity_No;			//奇偶校验
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//硬件流控制使能或失能
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;				   //发送和接收使能
  USART_Init(USART1, &USART_InitStructure);
   

  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启中断
  USART_Cmd(USART1, ENABLE);                    //使能串口 
}

//串口1中断服务程序
void USART1_IRQHandler(void)  
	{
	u8 dat;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //若接收数据寄存器满,接收中断(接收到的数据必须是0x0d 0x0a结尾)
		{
		dat =USART_ReceiveData(USART1);			//读取接收到的数据
		if((USART_RX_STA&0x80)==0)				//接收未完成
			{
			if(USART_RX_STA&0x40)//接收到了0x0d
				{
				if(dat!=0x0a)USART_RX_STA=0;//接收错误,重新开始
				else USART_RX_STA|=0x80;	//接收完成了 
				}
			else //还没收到0X0D
				{	
				if(dat==0x0d)USART_RX_STA|=0x40;
				else
					{
					USART_RX_BUF[USART_RX_STA&0X3F]=dat ;
					USART_RX_STA++;
					if(USART_RX_STA>63)USART_RX_STA=0;//接收数据错误,重新开始接收	  
					}		 
				}
			}   		 
     } 
} 

//	u8 t;u8 len;u16 times=0;
//uart_init(9600);//串口初始化为9600
//	 	if(USART_RX_STA&0x80)
//		{					   
//			len=USART_RX_STA&0x3f;//得到此次接收到的数据长度
//			printf("\n您发送的消息为:\n");
//			for(t=0;t<len;t++)
//			{
//				USART1->DR=USART_RX_BUF[t];
//				while((USART1->SR&0X40)==0);//等待发送结束
//			}
//			printf("\n\n");//插入换行
//			USART_RX_STA=0;
//		}
//		else
//		{
//			times++;
//			if(times%200==0)printf("请输入数据,以回车键结束\n");  
//			delay_ms(1000);   
//		}	

/* STM32向串口1发送1字节*/
//void Uart1_PutChar(u8 ch)
//{
//  USART_SendData(USART1, (u8) ch);
//  while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
//}
