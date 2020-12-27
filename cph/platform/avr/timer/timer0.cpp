#include "timer.h"
#include <void/interrupt.h>
#include <cph/platform/interrupt_tag_define.h>
using namespace cph::Private;
#if defined(CPH_ENABLE_TIMER0)
VOID_ISR(CPH_INT_TIMER0_OCRA) {
	PROVIDE_ENTRY(CPH_INT_TIMER0_OCRA);
	timer0::THwTimer0::IntOCRA_Handler();
}
#endif


