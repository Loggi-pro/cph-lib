#pragma once
#include <void/atomic.h>
#include <cph/ioreg.h>
#include <void/chrono.h>
namespace cph {
	namespace Private {
		class wdt {
				#ifdef __AVR_ATmega328P__
				IO_REG_WRAPPER(WDTCSR, Wdtsr, uint8_t);
				enum WdtsrBits {
					Wdce = WDCE,

				};
				#else
				IO_REG_WRAPPER(WDTCR, Wdtsr, uint8_t);
				enum WdtsrBits {
					Wdce = WDTOE,
				};
				#endif

				enum Period {
					Period15ms = 0,
					Period30ms = 1 << WDP0,
					Period60ms = 1 << WDP1,
					Period120ms = (1 << WDP0) | (1 << WDP1),
					Period250ms = (1 << WDP2),
					Period500ms = (1 << WDP2) | (1 << WDP0),
					Period1s = (1 << WDP2) | (1 << WDP1),
					Period2s = (1 << WDP0) | (1 << WDP1) | (1 << WDP0)
				};
				static Period PeriodFromMsec(uint32_t periodMSec) {
					Period result = Period15ms;

					if (periodMSec > 1000) {
						result = Period2s;
					} else if (periodMSec > 500) {
						result = Period1s;
					} else if (periodMSec > 250) {
						result = Period500ms;
					} else if (periodMSec > 120) {
						result = Period250ms;
					} else if (periodMSec > 60) {
						result = Period120ms;
					} else if (periodMSec > 30) {
						result = Period60ms;
					} else if (periodMSec > 15) {
						result = Period30ms;
					}

					return result;
				}

			public:

				static void start(uint32_t periodMSec) {
					Period period = PeriodFromMsec(periodMSec);
					Wdtsr::Set(period | (1 << WDE));
					reset();
				}

				template <class T>
				constexpr static void start(const vd::chrono::milli_t m) {
					Period period = PeriodFromMsec(m.count());
					Wdtsr::Set(period | (1 << WDE));
					reset();
				}




				static void disable() {
					ATOMIC_BLOCK() {
						/* Clear WDRF in MCUSR */
						reset();
						MCUSR &= ~(1 << WDRF);
						Wdtsr::Or((1 << WdtsrBits::Wdce) | (1 << WDE));
						Wdtsr::Set(0);
					}
				}

				static void reset() {
					asm("wdr");
				}

				static bool causeReset() {
					return false; //TODO
				}
		};
	}
}