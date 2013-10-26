#include "sys.h"
#include "usart.h"

//�������´���,֧��printf����,������Ҫѡ��use MicroLIB
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

/*ʹ��microLib�ķ���*/
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
 
u8 USART_RX_BUF[64];     //���ջ���,���64���ֽ�,�洢����ASCII�룡һ�����ֻ���ĸ���ַ�Ϊһ�ֽ�
//����״̬
//bit7��������ɱ�־
//bit6�����յ�0x0d
//bit5~0�����յ�����Ч�ֽ���Ŀ
u8 USART_RX_STA=0;       //����״̬���

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

   //Usart1 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;

	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);
  
   //USART ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;					//bps
	USART_InitStructure.USART_WordLength = USART_WordLength_8b; //������������λ��
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		//ֹͣλ��Ŀ
	USART_InitStructure.USART_Parity = USART_Parity_No;			//��żУ��
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//Ӳ��������ʹ�ܻ�ʧ��
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;				   //���ͺͽ���ʹ��
  USART_Init(USART1, &USART_InitStructure);
   

  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�����ж�
  USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ��� 
}

//����1�жϷ������
void USART1_IRQHandler(void)  
	{
	u8 dat;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //���������ݼĴ�����,�����ж�(���յ������ݱ�����0x0d 0x0a��β)
		{
		dat =USART_ReceiveData(USART1);			//��ȡ���յ�������
		if((USART_RX_STA&0x80)==0)				//����δ���
			{
			if(USART_RX_STA&0x40)//���յ���0x0d
				{
				if(dat!=0x0a)USART_RX_STA=0;//���մ���,���¿�ʼ
				else USART_RX_STA|=0x80;	//��������� 
				}
			else //��û�յ�0X0D
				{	
				if(dat==0x0d)USART_RX_STA|=0x40;
				else
					{
					USART_RX_BUF[USART_RX_STA&0X3F]=dat ;
					USART_RX_STA++;
					if(USART_RX_STA>63)USART_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
					}		 
				}
			}   		 
     } 
} 

//	u8 t;u8 len;u16 times=0;
//uart_init(9600);//���ڳ�ʼ��Ϊ9600
//	 	if(USART_RX_STA&0x80)
//		{					   
//			len=USART_RX_STA&0x3f;//�õ��˴ν��յ������ݳ���
//			printf("\n�����͵���ϢΪ:\n");
//			for(t=0;t<len;t++)
//			{
//				USART1->DR=USART_RX_BUF[t];
//				while((USART1->SR&0X40)==0);//�ȴ����ͽ���
//			}
//			printf("\n\n");//���뻻��
//			USART_RX_STA=0;
//		}
//		else
//		{
//			times++;
//			if(times%200==0)printf("����������,�Իس�������\n");  
//			delay_ms(1000);   
//		}	

/* STM32�򴮿�1����1�ֽ�*/
//void Uart1_PutChar(u8 ch)
//{
//  USART_SendData(USART1, (u8) ch);
//  while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
//}
