#include "timer.h"
#include <void/interrupt.h>
#include <cph/platform/interrupt_tag_define.h>
using namespace cph::Private;
#if defined(CPH_ENABLE_TIMER1)
VOID_ISR(CPH_INT_TIMER1_OCRA) {
	PROVIDE_ENTRY(CPH_INT_TIMER1_OCRA);
	timer1::THwTimer1::IntOCRA_Handler();
}
#endif



