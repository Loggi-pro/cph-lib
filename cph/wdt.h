#pragma once

#ifdef _AVR
	#include "platform/avr/watchdog/platform_watchdog.h"
#elif defined(_ARM)
	#include "platform/stm32/watchdog/CortexM4/platform_watchdog.h"
#endif

namespace cph {

	class wdt {
		public:
			static void start(uint32_t millisec) {
				cph::Private::wdt::start(millisec);
			}
			template <typename Period>
			constexpr static void start(const vd::chrono::any_t<Period> milli) {
				cph::Private::wdt::template start<Period>(milli);
			}

			static void disable() {
				cph::Private::wdt::disable();
			}

			static void reset() {
				cph::Private::wdt::reset();
			}

			static bool causeReset() {
				return cph::Private::wdt::causeReset();
			}
	};
}