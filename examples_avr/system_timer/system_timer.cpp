#include <void/chrono.h>
#include <cph/timer.h>
#include <void/interrupt.h>
#include <void/atomic.h>
#include <cph/basic_usart.h>
#include <cph/system_counter.h>
#include <cph/iostream.h>





bool flag_send = false;



void init(vd::chrono::micro_t m) {
	if (DDRB == 0) {
		cph::SystemCounter::initCustom<cph::TIMER_1, F_CPU>(m);
	} else {
		cph::SystemCounter::initCustom<cph::TIMER_1, F_CPU>(400_ms);
	}
}
int main() {
	cph::SystemCounter::initCustom<cph::TIMER_1, F_CPU>(1_ms); // OCR = 16000, PRESCALAR = 1
	cph::SystemCounter::initCustom<cph::TIMER_1, F_CPU>(118_ms); // OCR = 25900, PRESCALAR = 64
	init(15_ms);//OCRVALUE=30000,PRESCALAR=8
	cph::SystemCounter::initPrecision<cph::Resolution_1ms, cph::TIMER_1, F_CPU>();
	cph::SystemCounter::enable<cph::TIMER_1>();
	vd::EnableInterrupts();
	DDRB |= 1 << PB5;
	cph::timer_t t;

	while (true) {
		if (t.isElapsed(1_s)) {
			cph::SystemCounter::disable<cph::TIMER_1>();
			cph::SystemCounter::enable<cph::TIMER_1>();
			flag_send = false;
			PORTB ^= (1 << PB5);
		}
	}

	return 0;
}