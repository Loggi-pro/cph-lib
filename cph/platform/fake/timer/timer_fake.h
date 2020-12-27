#pragma once
#include <void/static_assert.h>
#include <void/assert.h>
#include <cph/timer/timer_types.h>
#include <void/chrono.h>
#include <void/meta.h>
#include <cph/platform/avr/timer/timer_helper.h>
namespace cph {
	namespace Private {

		template <TimerPrescalar P>
		struct FakeTimerPrescalarData {};

		template <TimerPrecision R, uint32_t FCPU>
		struct FakeTimerPrecisionInfo {};

		struct TFakeTimerHelper;


		struct TFakeTimer {
				friend struct TFakeTimerHelper;
				typedef void (*timerTickCallback_t)();
			private:
				static inline bool _isInited = false;
				static inline uint32_t _ocrValue = 0;
				static inline TimerPrescalar _currentPrescalar = TimerPrescalar::Stopped;
				static inline bool _isEnabled = false;
				static inline volatile timerTickCallback_t _cbOCRA_COMP = nullptr;

			public:
				enum {
					maxCounterValue = 255
				};

				static void IntOCRA_Handler() {
					if (_cbOCRA_COMP != nullptr) { _cbOCRA_COMP(); }
				}

				template <TimerPrecision R>
				static uint16_t tickPeriod_us() {
					return FakeTimerPrecisionInfo<R, 0>::usDivider();
				}



				enum TimerMode {

				};



				static void reset() {
					_isInited = false;
					_isEnabled = false;
				}

				static void setMode(TimerMode) {
				}

				template <TimerPrescalar P>
				static void setPrescalar() {
					_currentPrescalar = P;
				}


				template <TimerPrecision R, uint32_t FCPU>
				static void initPrecisionTimer(timerTickCallback_t cbTick) {
					_isInited = true;
					_ocrValue = FakeTimerPrecisionInfo<R, FCPU>::OcrValue;
					_cbOCRA_COMP = cbTick;
					setPrescalar<FakeTimerPrecisionInfo<R, FCPU>::Prescalar>();
				}

				template <TimerPrescalar P, class Period, uint32_t time, uint32_t FCPU>
				static void initCustomTimer(timerTickCallback_t cbTick) {
					_isInited = true;
					typedef TimerPrescalarValue<P> prescalar;
					//const uint32_t ocrValue = (usTickTime * F_CPU) / 1000000 / (prescalar::value);
					_cbOCRA_COMP = cbTick;
					_ocrValue = ((FCPU) / (1000UL * Period::Denom) * Period::Num * time) /
					            (prescalar::value);
					assert(_ocrValue <= maxCounterValue);
					//Ocra::Set(Void::lowByte(ocrValue));
					//setMode(TimerMode::Ctc);
					setPrescalar<P>();
				}

				template <TimerPrescalar P, uint32_t FCPU, class Period>
				static void initCustomTimer(vd::chrono::any_t<Period> tickTime, timerTickCallback_t cbTick) {
					typedef TimerPrescalarValue<P> prescalar;
					//const uint32_t ocrValue = (tickTime * F_CPU / 1000000) / (prescalar::value);
					_cbOCRA_COMP = cbTick;
					_ocrValue = ((FCPU) / (1000UL * Period::Denom) * Period::Num * tickTime.count()) /
					            (prescalar::value);
					assert(_ocrValue <= maxCounterValue); //make Prescalar bigger!
					//Ocra::Set(Void::lowByte(ocrValue));
					//setMode(TimerMode::Ctc);
					setPrescalar<P>();
				}
				static void enableTimer() {
					_isEnabled = true;
				}

				static void disableTimer() {
					_isEnabled = false;
				}
				template <uint32_t FCPU, class Period>
				constexpr static void initCustomTimer(const vd::chrono::any_t<Period>& tickTime,
				                                      timerTickCallback_t /*cbTick*/, bool /*IsEnable*/) {
					auto [prescalar,
					      ocrValue] = cph::Private::timer::calculatePrescalar <  maxCounterValue, FCPU, Period>(tickTime);
					assert(ocrValue <= maxCounterValue); //make Prescalar bigger!
					_ocrValue = ocrValue;
					_currentPrescalar = prescalar;
				}

		};


		template <>
		struct FakeTimerPrescalarData<Stopped> {
			enum {
				Bits = 0,
			};
		};

		template <>
		struct FakeTimerPrescalarData<Prescalar_1> {
			enum {
				Value = 1,
			};
		};
		template <>
		struct FakeTimerPrescalarData<Prescalar_8> {
			enum {
				Bits = 0,
			};
		};
		template <>
		struct FakeTimerPrescalarData<Prescalar_64> {
			enum {
				Bits = 0,
			};
		};
		template <>
		struct FakeTimerPrescalarData<Prescalar_256> {
			enum {
				Bits = 0,
			};
		};

		template <>
		struct FakeTimerPrescalarData<Prescalar_1024> {
			enum {
				Bits = 0,
			};
		};



		template <uint32_t FCPU>
		struct FakeTimerPrecisionInfo<Resolution_1ms, FCPU> {
			static const TimerPrescalar Prescalar = TimerPrescalar::Prescalar_64;
			enum {
				OcrValue = FCPU / TimerPrescalarValue<Prescalar>::value / 1000,
				usDivider = 1000
			};
			VOID_STATIC_ASSERT(OcrValue <= 255);
		};
		template <uint32_t FCPU>
		struct FakeTimerPrecisionInfo<Resolution_100us, FCPU> {
			static const TimerPrescalar Prescalar = TimerPrescalar::Prescalar_8;

			enum {
				OcrValue = FCPU / TimerPrescalarValue<Prescalar>::value / 10000,
				usDivider = 100
			};
			VOID_STATIC_ASSERT(OcrValue <= 255);
		};
		template <uint32_t FCPU>
		struct FakeTimerPrecisionInfo<Resolution_10us, FCPU> {
			static const TimerPrescalar Prescalar = TimerPrescalar::Prescalar_1;

			enum {
				OcrValue = FCPU / TimerPrescalarValue<Prescalar>::value / 100000,
				usDivider = 10
			};
			VOID_STATIC_ASSERT(OcrValue <= 255);
		};



		struct TFakeTimerHelper {
			static bool isInited() {
				return TFakeTimer::_isInited;
			}
			static void deinit() {
				TFakeTimer::reset();
			}

			static bool isEnabled() {
				return TFakeTimer::_isEnabled;
			}

			static uint32_t getOcrValue() {
				return TFakeTimer::_ocrValue;
			}
			static TimerPrescalar getCurrentPrescalar() {
				return TFakeTimer::_currentPrescalar;
			}

		};


	}
}