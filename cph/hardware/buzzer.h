#pragma once
#include <cph/timer.h>
#include <cph/pwm.h>
namespace cph {
	template <cph::TTimerNumber TIMER_N, cph::PwmChannel channel>
	class TBuzzer {
		private:
			static bool inline _isOn = false;
			static bool inline _isWork = false;
			static inline cph::timer_t _timer;
		public:

			static void init() {
				cph::TPwmPeriod::initPwmPeriod<TIMER_N, channel, 1000>(false, true);
				cph::TPwmPeriod::disable<TIMER_N, channel>();
			}
			static void enable() {
				cph::TPwmPeriod::enable<TIMER_N, channel>();
				_isOn = true;
				_isWork = true;
			}
			static void disable() {
				cph::TPwmPeriod::disable<TIMER_N, channel>();
				_isOn = false;
				_isWork = false;
			}

			static bool isEnable() {
				return _isWork;
			}
			template <typename literalTime1, typename literalTime2>
			constexpr static void pulse(literalTime1 onTime, literalTime2 offTime) {
				_isWork = true;

				if (_isOn) {
					if (_timer.isElapsed(onTime)) {
						cph::TPwmPeriod::disable<TIMER_N, channel>();
						_isOn = false;
					}
				} else {
					if (_timer.isElapsed(offTime)) {
						_isOn = true;
						cph::TPwmPeriod::enable<TIMER_N, channel>();
					}
				}
			}

	};
}