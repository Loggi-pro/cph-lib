#include "platform_uart.h"
#include <void/interrupt.h>
#include <cph/basic_usart.h>
#include <cph/platform/interrupt_tag_define.h>
extern "C" {
	#if defined(CPH_HAS_UART1) & defined(CPH_ENABLE_UART1)
	VOID_ISR(CPH_INT_USART1_RXC) {
		PROVIDE_ENTRY(CPH_INT_USART1_RXC);
		cph::Usart1::IntRxHandler();
	}

	VOID_ISR(CPH_INT_USART1_TXE) {
		PROVIDE_ENTRY(CPH_INT_USART1_TXE);
		cph::Usart1::IntTxEmptyHandler();
	}
	#endif
}




