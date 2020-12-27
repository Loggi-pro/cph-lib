#include "stm32_exti.h"
#include <void/bits.h>
using namespace cph;
exti::isr_handle_t exti::_handlers[16] = {};

extern "C" {
	void EXTI0_IRQHandler() {
		vd::setBit(EXTI->PR,0);
		exti::call(0);
	}

	void EXTI1_IRQHandler(void) {
		vd::setBit(EXTI->PR,1);
		exti::call(1);
	}

	void EXTI2_TSC_IRQHandler(void) {
		vd::setBit(EXTI->PR,2);
		exti::call(2);
	}

	void EXTI3_IRQHandler(void) {
		vd::setBit(EXTI->PR,3);
		exti::call(3);
	}
	void EXTI4_IRQHandler(void) {
		vd::setBit(EXTI->PR,4);
		exti::call(4);
	}
	void EXTI9_5_IRQHandler(void) {
		uint32_t status = (EXTI->PR);
		uint32_t mask  = (1 << 5);

		for	(uint8_t i = 5; i < 10; ++i) {
			if ((status & mask)) {
				vd::setBit(EXTI->PR, i);
				exti::call(i);
			}

			mask <<= 1;
			// mask |= 0x01;
		}
	}
	void EXTI15_10_IRQHandler(void) {
		uint32_t status = (EXTI->PR);
		uint32_t mask  = (1 << 10);

		for	(uint8_t i = 10; i < 15; ++i) {
			if ((status & mask)) {
				vd::setBit(EXTI->PR, i);
				exti::call(i);
			}

			mask <<= 1;
			// mask |= 0x01;
		}
	}
}