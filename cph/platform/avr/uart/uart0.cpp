#include "uart.h"
#include <void/interrupt.h>
#include <cph/platform/interrupt_tag_define.h>
extern "C" {
	#if defined(CPH_HAS_UART0) & defined(CPH_ENABLE_UART0)
	VOID_ISR(CPH_INT_USART0_RXC) {
		PROVIDE_ENTRY(CPH_INT_USART0_RXC);
		cph::Usart0::IntRxHandler();
	}

	VOID_ISR(CPH_INT_USART0_TXE) {
		PROVIDE_ENTRY(CPH_INT_USART0_TXE);
		cph::Usart0::IntTxEmptyHandler();
	}

	#endif
}




