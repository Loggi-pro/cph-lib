#pragma once
#include <void/chrono.h>
#include <void/static_assert.h>
#include <cph/timer/timer_types.h>
#include <void/delay.h>
#ifdef  _ARM
#include <cph/platform/stm32/timer/platform_timer.h>



namespace cph::stm32 {
	struct TWakeUpTimer {

		template <class literalTime>
		static void initWakeUpEvery(timerTickCallback_t cbWakeUp) {
			cph::Private::timer_rtc::TTimer::initWakeUpEvery<literalTime>(cbWakeUp);
		}
		#ifdef _VOID_NATIVE_CPP17_CONSTEXPR_SUPPORT
		template <class T>
		constexpr static void initWakeUpEvery(T literalTime, timerTickCallback_t cbWakeUp) {
			cph::Private::timer_rtc::TTimer::initWakeUpEvery<T>(literalTime, cbWakeUp);
		}
		#endif

		static void disable() {
			cph::Private::timer_rtc::TTimer::disable();
		}


	};

}
#endif