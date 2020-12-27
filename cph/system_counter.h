#pragma once
#include <void/atomic.h>
#include <void/chrono.h>
#include <void/static_assert.h>
#include <cph/timer/timer_types.h>
#include <cph/timer/selector.h>
#include <void/type_traits.h>

namespace cph {



	class TCounter {
		private:
			TCounter() {}
		public:
			typedef void(*cbOnTimerEnd_t)();

			template <TimerPrecision P, TTimerNumber N, uint32_t FCPU = F_CPU>
			static void initPrecision(cbOnTimerEnd_t cb, bool IsEnable = true) {
				typedef typename Private::TimerSelector<N>::selected Hardware;
				Hardware::template initPrecisionTimer<P, FCPU>(cb, IsEnable);
				enable<N>();
			}
			/*
			template <class LiteralTime, TTimerNumber N, uint32_t FCPU = F_CPU>
			constexpr static void initCustom(cbOnTimerEnd_t cb) {
				typedef typename Private::TimerSelector<N>::selected Hardware;
				Hardware::reset();
				Hardware::template
				initCustomTimer<::cph::TimerPrescalar(FindedPrescalar), typename LiteralTime::PeriodInfo, LiteralTime::static_value, FCPU>
				(cb);
			}
			*/
			template <TTimerNumber N, uint32_t FCPU = F_CPU, class Period>
			constexpr static void initCustom(vd::chrono::any_t<Period> tickTime, cbOnTimerEnd_t cb,
			                                 bool IsEnable = true) {
				typedef typename Private::TimerSelector<N>::selected Hardware;
				Hardware::template initCustomTimer <FCPU>(tickTime, cb, IsEnable);
				enable<N>();
			}
			template<TTimerNumber N >
			static void enable() {
				typedef typename Private::TimerSelector<N>::selected Hardware;
				Hardware::enableTimer();
			}
			template<TTimerNumber N >
			static void disable() {
				typedef typename Private::TimerSelector<N>::selected Hardware;
				Hardware::disableTimer();
			}
			template<TTimerNumber N >
			static void reset() {
				typedef typename Private::TimerSelector<N>::selected Hardware;
				Hardware::reset();
			}
	};
	template <TTimerNumber N, TimerPrecision P, uint32_t FCPU = F_CPU>
	struct TTimerConfig {
		static const inline TTimerNumber Number = N;
		static const inline TimerPrecision Precision = P;
		static const inline uint32_t FREQUENCY = FCPU;
	};
	class SystemCounter {
		public:
			typedef uint32_t counter_t;
		private:
			SystemCounter() = delete;

			static inline counter_t _addCount = 0;  //tick in us
			volatile static inline counter_t _microSec = 0;

			static void _IntHandler() {
				#ifdef _AVR
				//single thread, in interrupt, interrupt disables=>nobody can interrupt
				_microSec += _addCount;
				#else
				vd::AtomicFetchAndAdd(&_microSec, _addCount);
				#endif
				#ifdef USE_HAL_DRIVER

				if (_microSec % 1000 == 0) {
					HAL_IncTick(); // in millisec
				}

				#endif
			}
			static void _reset(uint32_t addValue) {
				ATOMIC_BLOCK(vd::ON_EXIT::RESTORE_STATE) {
					_microSec = 0;
					_addCount = addValue;
				}
			}

			template<TTimerNumber N>
			using ParentSpy = typename Private::TimerSelector<N>::fakehelper;

			template <TTimerNumber N>
			struct SpyHelper : public ParentSpy<N> {
				public:
					static void setTime(uint32_t microSec) {
						_microSec = microSec;
					}
					static void incTime(uint32_t value) {
						_microSec += value;
					}
					static void decTime(uint32_t value) {
						_microSec -= value;
					}
			};


		public:
			static vd::chrono::micro_t  time_from_start() {
				return vd::chrono::micro_t(_microSec);
			}
			template <TTimerNumber N>
			using Spy = SpyHelper<N>;
			template <TimerPrecision P, TTimerNumber N, uint32_t FCPU = F_CPU>
			static void initPrecision() {
				typedef typename Private::TimerSelector<N>::selected Hardware;
				_reset(Hardware::template tickPeriod_us<P>());
				TCounter::template initPrecision <P, N, FCPU>(&_IntHandler, true);
				enable<N>();
			}

			template <class CONFIG>
			static void initPrecision() {
				initPrecision<CONFIG::Precision, CONFIG::Number, CONFIG::FREQUENCY>();
			}

			template <TTimerNumber N, uint32_t FCPU = F_CPU, class Period>
			constexpr static void initCustom(vd::chrono::any_t<Period> tickTime) {
				//typedef typename Private::TimerSelector<N>::selected Hardware;
				_reset(vd::chrono::micro_t(tickTime).count());
				TCounter::template initCustom <N, FCPU>(tickTime, &_IntHandler, true);
				enable<N>();
			}

			template <class CONFIG, class Period>
			constexpr static void initCustom(vd::chrono::any_t<Period> tickTime) {
				initCustom<CONFIG::Number, CONFIG::FREQUENCY, Period>(tickTime);
			}

			template<TTimerNumber N >
			static void enable() {
				TCounter::template enable<N>();
				#ifdef USE_HAL_DRIVER
				HAL_ResumeTick();
				#endif
			}
			template<TTimerNumber N >
			static void disable() {
				TCounter::template disable<N>();
				#ifdef USE_HAL_DRIVER
				HAL_SuspendTick();
				#endif
			}


			static counter_t atomic_extract_value() {
				if constexpr (vd::is_same_v<counter_t, uint16_t>) {
					return atomic_extract_value_u16();
				} else if constexpr (vd::is_same_v<counter_t, uint32_t>) {
					return atomic_extract_value_u32();
				}
			}

			static counter_t atomic_extract_value_u16() {
				counter_t res = vd::hiByte((uint16_t)_microSec);
				uint8_t low = vd::lowByte(_microSec);
				uint8_t newHiByte = vd::hiByte((uint16_t)_microSec);

				if (newHiByte > res) {
					low = vd::lowByte((uint16_t)_microSec);
					res = vd::makeWord(low, newHiByte);
				} else {
					res = vd::makeWord(low, (uint8_t)res);
				}

				return res;
			}
			static counter_t atomic_extract_value_u32() {
				counter_t res;
				ATOMIC_BLOCK(vd::ON_EXIT::FORCE_ON) {
					res = _microSec;
				}
				return res;
			}
			static counter_t ticks_us() {
				//return atomic_extract_value();// atomic_extract_value<counter_t>();
				//vd::DisableInterrupts();
				counter_t res = atomic_extract_value();
				//vd::EnableInterrupts();
				return res;
			}
	};



}
