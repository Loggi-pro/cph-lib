#pragma once
#include <void/static_assert.h>
#include <void/assert.h>
#include <void/chrono.h>
#include <cph/timer/timer_types.h>

#include <void/delay.h>
#include <vector>
#include <algorithm>
#ifndef F_CPU
	#error F_CPU not defined for <timer.h> simulation for win32 project
#endif

#define CPH_HAS_TIMER_CORE

#ifdef VOID_COMPILER_MSVC
#include <concrt.h>
#elif defined(VOID_COMPILER_GCC)
#include <pthread.h>


namespace concurrency {
	//NOT IMPLEMENTED IN MINGW
	class critical_section{



	public:
		critical_section(){
		}
		void lock(){
		}
		void unlock(){
		}
	};
}
#endif


namespace cph::Private {

	class WindowsTimer {
		public:
			static inline volatile bool _isCounterWork = false;
			static inline volatile bool _isTerminated = true;
			static inline concurrency::critical_section _accessToHandlers{};
			typedef void(*func_ptr)();
			static inline std::vector<func_ptr> _handlers{};
			void static ExecuteThreadAdapter(void*) {
				while (_isCounterWork) {
					_isTerminated = false;

					while (true) {
						vd::delay_us<10>();

						for (func_ptr x : _handlers) {
							if (x != nullptr) {
								x();
							}
						}
					}
				}

				_isTerminated = true;
			}
			void static addHandler(func_ptr p) {
				auto pos = std::find(_handlers.begin(), _handlers.end(), p);

				if (pos == _handlers.end()) {
					_accessToHandlers.lock();
					_handlers.push_back(p);
					_accessToHandlers.unlock();
				}
			}
			static void removeHandler(func_ptr p) {
				auto pos = std::find(_handlers.begin(), _handlers.end(), p);

				if (pos != _handlers.end()) {
					_accessToHandlers.lock();
					_handlers.erase(pos);
					_accessToHandlers.unlock();
				}
			}


			static void enable() {
				if (!_isCounterWork) {
					_isCounterWork = true;
					_beginthread(ExecuteThreadAdapter, 0, NULL);
				}
			}
			static void disable() {
				_isCounterWork = false;

				while (!_isTerminated) { continue; }
			}
	};


	struct TTimerHelperStub {


	};

	template <TimerPrecision R>
	struct PrecisionInfo;
	template <>
	struct PrecisionInfo<Resolution_1ms> {
		//static const uint32_t divider = 1000UL;
		static const uint32_t divider = 1000UL;
	};
	template <>
	struct PrecisionInfo<Resolution_100us> {
		//static const uint32_t divider = 10000UL;
		static const uint32_t divider = 100UL;
	};
	template <>
	struct PrecisionInfo<Resolution_10us> {
		//static const uint32_t divider = 100000UL;
		static const uint32_t divider = 10UL;
	};
	namespace timer_core {

		struct THwTimerCore {
			typedef void (*timerTickCallback_t)();
			inline static volatile timerTickCallback_t _cbTick = nullptr;
			typedef u32 counter_t;
			inline static uint32_t _usDivider = 0;
			inline static volatile counter_t _counter = 0;

			enum TimerMode {
				Normal = 0,
				FastPwm,
				Ctc,
			};

			template < TimerPrecision P>
			static counter_t tickPeriod_us() {
				return PrecisionInfo<P>::divider;
			}

			static void reset() {
				//TODO SIM IMPLEMENTATION
			}

			static void setMode(TimerMode) {
				//TODO SIM IMPLEMENTATION
			}
			static void _timerCallback() {
				_counter = (_counter + 10) % _usDivider;

				if (_counter == 0) {
					_cbTick();
				}
			}

			template <TimerPrecision R, uint32_t FCPU>
			static void initPrecisionTimer(timerTickCallback_t cbTick, bool IsEnable) {
				disableTimer();
				_counter = 0;
				_usDivider = PrecisionInfo<R>::divider;
				_cbTick = cbTick;

				if (IsEnable) {
					enableTimer();
				} else {
					disableTimer();
				}
			}


			template <uint32_t FCPU, class Period>
			static void initCustomTimer(vd::chrono::any_t<Period> tickTime, timerTickCallback_t cbTick) {
				disableTimer();
				_cbTick = cbTick;
				vd::chrono::micro_t micro(tickTime);
				_counter = 0;
				_usDivider = tickTime;
				enableTimer();
			}

			static void enableTimer() {
				WindowsTimer::addHandler(&_timerCallback);
				WindowsTimer::enable();
			}

			static void disableTimer() {
				WindowsTimer::removeHandler(&_timerCallback);
				WindowsTimer::disable();
			}

		};
	}
}