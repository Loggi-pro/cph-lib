#pragma once
#include <void/chrono.h>
#include <void/static_assert.h>
#include <cph/timer/timer_types.h>
#include <void/atomic.h>
#ifdef _STM8

#elif defined _AVR
	#include <cph/platform/avr/timer/timer.h>
#elif defined WIN32
	#include <cph/platform/win32/timer/timer_win32.h> //simulation
#elif defined _ARM
	#include <cph/platform/stm32/timer/platform_timer.h>
#endif
#include <cph/platform/fake/timer/timer_fake.h>
#include <cph/timer/selector.h>

namespace cph {



	class TPwm {
			TPwm() = delete;
		public:
			template <TTimerNumber N, cph::PwmChannel channel, TimerPrescalar prescalar, uint32_t FCPU = F_CPU>
			static void initPwm() {
				typedef typename Private::TimerSelector<N>::selected Hardware;
				Hardware::template
				initPwm<channel, Hardware::DirectPwm, prescalar, FCPU>
				(0);
			}

			template<TTimerNumber N, cph::PwmChannel channel>
			static void enable() {
				typedef typename Private::TimerSelector<N>::selected Hardware;
				Hardware::template enablePwm<channel>();
			}
			template<TTimerNumber N, cph::PwmChannel channel>
			static void disable() {
				typedef typename Private::TimerSelector<N>::selected Hardware;
				Hardware::disablePwm();
			}
			template<TTimerNumber N, cph::PwmChannel channel>
			static void setValuePercent(uint8_t value) { //from 0 to 100
				typedef typename Private::TimerSelector<N>::selected Hardware;
				Hardware::template pwmSetValuePercent<channel>(value);
			}
	};

	class TPwmPeriod {
			TPwmPeriod() = delete;
		public:
			template <TTimerNumber N, cph::PwmChannel channel, uint32_t Period, uint32_t FCPU = F_CPU >
			static void initPwmPeriod(bool start, bool isInverted) {
				typedef typename Private::TimerSelector<N>::selected Hardware;
				Hardware::template
				initPwmPeriod<channel, Period, FCPU>(start, isInverted);
			}

			template<TTimerNumber N, cph::PwmChannel channel>
			static void enable() {
				typedef typename Private::TimerSelector<N>::selected Hardware;
				Hardware::template enablePwmPeriod<channel>();
			}
			template<TTimerNumber N, cph::PwmChannel channel>
			static void disable() {
				typedef typename Private::TimerSelector<N>::selected Hardware;
				Hardware::template disablePwmPeriod<channel>();
			}
	};

}
