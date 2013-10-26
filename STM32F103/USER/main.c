#include "sys.h"
#include "delay.h"
#include "usart.h"

int main ()
{	

  SystemInit(); 			
	Delay_init(72);	    	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	
	uart_init(9600);		
	
	while (1)
	 {
	 ;
	 }

}
