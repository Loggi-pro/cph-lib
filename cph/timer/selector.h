#pragma once
#ifdef _STM8

#elif defined _AVR
	#include <cph/platform/avr/timer/timer.h>
#elif defined WIN32
	#include <cph/platform/win32/timer/timer_win32.h> //simulation
#elif defined _ARM
	#include <cph/platform/stm32/timer/platform_timer.h>
#endif
#include <cph/platform/fake/timer/timer_fake.h>

namespace cph {
	enum TTimerNumber {
		TIMER_FAKE = -1,
		#ifdef CPH_ENABLE_TIMER0
		TIMER_0 = 0,
		#endif
		#ifdef CPH_ENABLE_TIMER1
		TIMER_1 = 1,
		#endif
		#ifdef CPH_ENABLE_TIMER2
		TIMER_2 = 2,
		#endif
		#ifdef CPH_ENABLE_TIMER_CORE
		TIMER_CORE = 255,
		#endif
		//default_timer
		#ifdef CPH_ENABLE_TIMER_CORE
		TIMER_DEFAULT = TIMER_CORE,
		#elif defined (CPH_ENABLE_TIMER0)
		TIMER_DEFAULT = TIMER_0
		#elif defined (CPH_ENABLE_TIMER1)
		TIMER_DEFAULT = TIMER_1
		#elif defined (CPH_ENABLE_TIMER2)
				TIMER_DEFAULT = TIMER_2
		#endif
	};



	namespace Private {
		template <TTimerNumber Number>
		struct TimerSelector {};


		template <>
		struct TimerSelector <TIMER_FAKE> {
			typedef typename cph::Private::TFakeTimer selected;
			typedef typename cph::Private::TFakeTimerHelper fakehelper;

		};


		#ifdef CPH_ENABLE_TIMER0

		template <>

		struct TimerSelector<TIMER_0 > {
			typedef typename cph::Private::timer0::THwTimer0  selected;
			typedef typename cph::Private::TTimerHelperStub fakehelper;
		};

		#endif
		#ifdef CPH_ENABLE_TIMER1
		template <>

		struct TimerSelector<TIMER_1 > {
			typedef cph::Private::timer1::THwTimer1 selected;
			typedef cph::Private::TTimerHelperStub  fakehelper;
		};
		#endif
		#ifdef CPH_ENABLE_TIMER2
		template <>

		struct TimerSelector<TIMER_2 > {
			typedef cph::Private::THwTimer2 selected;
			typedef cph::Private::TTimerHelperStub fakehelper;
		};
		#endif
		#ifdef CPH_ENABLE_TIMER_CORE
		template <>
		struct TimerSelector<TIMER_CORE > {
			typedef cph::Private::timer_core::THwTimerCore selected;
			typedef cph::Private::TTimerHelperStub fakehelper;
		};
		#endif
	}

}

