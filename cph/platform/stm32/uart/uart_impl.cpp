#include "platform_uart.h"


extern "C"{
	#ifdef CPH_HAS_UART1 
void USART1_IRQHandler(void)
{
	cph::Private::Usart1::IntHandler();
}

#endif

#ifdef CPH_HAS_UART3 
void USART3_IRQHandler(void)
{
	cph::Private::Usart3::IntHandler();
}


#endif

#ifdef CPH_HAS_UART4
void UART4_IRQHandler(void)
{
	cph::Private::Usart4::IntHandler();
}

#endif

}




