#pragma once
#include <void/static_assert.h>
#include <void/assert.h>
#include <cph/ioreg.h>
#include <cph/timer/timer_types.h>
#include "pwm_selector.h"
#include <void/chrono.h>
#include <void/atomic.h>
#include <cph/gpio.h>
#include <void/meta.h>
#include <cph/platform/avr/timer/timer_helper.h>
namespace cph {
	namespace Private {
		struct TTimerHelperStub {
		};
	}
	enum PwmChannel {
		Channel_0,
		Channel_1
	};
}

#define  DECLARE_TIMER_PRESCALAR(TYPE,x,value) template <>\
	struct TYPE<x> {\
		enum {\
			Bits = value,\
		};\
	}

#if defined (__AVR_ATmega168P__) || defined(__AVR_ATmega328P__)

#define CPH_HAS_TIMER0

#define CPH_INT_TIMER0_OCRA TIMER0_COMPA_vect

#define CPH_HAS_TIMER1
#define CPH_INT_TIMER1_OCRA TIMER1_COMPA_vect
namespace cph {
	namespace Private {


		namespace timer0 {



			template <TimerPrescalar P>
			struct Timer0PrescalarData;
			DECLARE_TIMER_PRESCALAR(Timer0PrescalarData, Stopped, 0);
			DECLARE_TIMER_PRESCALAR(Timer0PrescalarData, Prescalar_1, 1 << CS00);
			DECLARE_TIMER_PRESCALAR(Timer0PrescalarData, Prescalar_8, 1 << CS01);
			DECLARE_TIMER_PRESCALAR(Timer0PrescalarData, Prescalar_64, (1 << CS00) | (1 << CS01));
			DECLARE_TIMER_PRESCALAR(Timer0PrescalarData, Prescalar_256, 1 << CS02);
			DECLARE_TIMER_PRESCALAR(Timer0PrescalarData, Prescalar_1024, (1 << CS00) | (1 << CS02));




			template <TimerPrecision R, uint32_t FCPU>
			struct PrecisionInfo;
			struct THwTimer0 {
					IO_REG_WRAPPER(TCCR0A, Tccra, uint8_t);
					IO_REG_WRAPPER(TCCR0B, Tccrb, uint8_t);
					IO_REG_WRAPPER(OCR0A, Ocra, uint8_t);
					IO_REG_WRAPPER(TIMSK0, Timsk, uint8_t);
					IO_REG_WRAPPER(TIFR0, Tifr, uint8_t);
					IO_REG_WRAPPER(TCNT0, Tcnt, uint8_t);
					static inline uint8_t _prescalarBits = Timer0PrescalarData<Stopped>::Bits;
					enum {
						maxCounterValue = 255
					};
					typedef u08 counter_t;
					enum TimerMode {
						Normal = 0,
						FastPwm = (1 << WGM00) | (1 << WGM01),
						Ctc = 1 << WGM01,
					};


					typedef void(*timerTickCallback_t)();
				private:
					static inline volatile timerTickCallback_t _cbOCRA_COMP = nullptr;

				public:

					template < TimerPrecision P>
					static uint16_t tickPeriod_us() {
						return PrecisionInfo<P, 0>::usDivider;
					}

					static void IntOCRA_Handler() {
						if (_cbOCRA_COMP != nullptr) { _cbOCRA_COMP(); }
					}

					static void reset() {
						Tcnt::Set(0);
					}

					static void setMode(TimerMode mode) {
						u16 currentMode = (u16)(Tccra::Get() | (Tccrb::Get() << 8));
						currentMode &= (uint16_t)(~((1 << WGM00) | (1 << WGM01) | ((1 << WGM02) << 8)));
						currentMode |= mode;
						Tccra::Set(vd::lowByte(currentMode));
						Tccrb::Set(vd::hiByte(currentMode));
					}

					template <TimerPrescalar P>
					static void setPrescalar() {
						typedef Timer0PrescalarData<P> info;
						Tccrb::AndOr(static_cast<uint8_t>(~((1 << CS00) | (1 << CS01) | (1 << CS02))), info::Bits);
					}
					static void setPrescalar(uint8_t bits) {
						Tccrb::AndOr(static_cast<uint8_t>(~((1 << CS00) | (1 << CS01) | (1 << CS02))), bits);
					}


					template <TimerPrecision R, uint32_t FCPU>
					static void initPrecisionTimer(timerTickCallback_t cbTick, bool IsEnable) {
						Ocra::Set(PrecisionInfo<R, FCPU>::OcrValue);
						setMode(TimerMode::Ctc);
						_cbOCRA_COMP = cbTick;
						_prescalarBits = Timer0PrescalarData<PrecisionInfo<R, FCPU>::Prescalar>::Bits;
						if (IsEnable) { setPrescalar<PrecisionInfo<R, FCPU>::Prescalar>(); } else { setPrescalar<Stopped>(); }
					}
					/*
					template <TimerPrescalar P, class Period, uint32_t time, uint32_t FCPU>
					static void initCustomTimer(timerTickCallback_t cbTick,bool IsEnable) {
						typedef TimerPrescalarValue<P> prescalar;
						//const uint32_t ocrValue = (usTickTime * FCPU) / 1000000 / (prescalar::value);
						const uint32_t ocrValue = ((FCPU) / (1000UL * Period::Denom) * Period::Num * time) /
						                          (prescalar::value);
						VOID_STATIC_ASSERT(ocrValue <= maxCounterValue); //make Prescalar bigger!
						Ocra::Set(vd::lowByte(ocrValue));
						setMode(TimerMode::Ctc);
						_cbOCRA_COMP = cbTick;
						_prescalarBits = Timer0PrescalarData<P>::Bits;
						if (IsEnable) { setPrescalar<P>(); } else { setPrescalar<Stopped>(); }
					}
					*/
					template <uint32_t FCPU, class Period>
					constexpr static void initCustomTimer(const vd::chrono::any_t<Period>& tickTime,
					                                      timerTickCallback_t cbTick, bool IsEnable) {
						auto [prescalar,
						      ocrValue] = cph::Private::timer::calculatePrescalar <maxCounterValue, FCPU, Period>(tickTime);
						assert(ocrValue <= maxCounterValue); //make Prescalar bigger!
						Ocra::Set(vd::lowByte(ocrValue));
						setMode(TimerMode::Ctc);
						_cbOCRA_COMP = cbTick;
						_prescalarBits = getTimerPrescalarBits< Timer0PrescalarData>(prescalar);
						if (IsEnable) { setPrescalar(_prescalarBits); } else { setPrescalar<Stopped>(); }

						/*constexpr TimerPrescalar FindedPrescalar
						    = (TimerPrescalar)(::cph::Private::PreselectorIterator<PrescalarValue, true, 0>::value);
						//TimerPrescalar P,
						typedef TimerPrescalarValue<FindedPrescalar> prescalar;
						//Too big interval, try to make interval lesser, or decrease MCU frequency (F_CPU)
						VOID_STATIC_ASSERT(::cph::TimerPrescalar(FindedPrescalar) != ::cph::TimerPrescalar::Stopped);
						//const uint32_t ocrValue = (tickTime * FCPU / 1000000) / (prescalar::value);
						const uint32_t ocrValue = ((FCPU) / (1000UL * Period::Denom) * Period::Num * tickTime.count()) /
						                          (prescalar::value);
						assert(ocrValue <= maxCounterValue); //make Prescalar bigger!
						Ocra::Set(vd::lowByte(ocrValue));
						setMode(TimerMode::Ctc);
						_cbOCRA_COMP = cbTick;
						_prescalarBits = Timer0PrescalarData<FindedPrescalar>::Bits;
						if (IsEnable) { setPrescalar(_prescalarBits); } else { setPrescalar<Stopped>(); }*/
					}

					static void enableTimer() {
						Tcnt::Set(0);
						Timsk::Or(1 << OCIE0A);
						setPrescalar(_prescalarBits);
					}

					static void disableTimer() {
						setPrescalar<Stopped>();
						Timsk::AndNot(1 << OCIE0A);
					}

			};
			template <uint32_t FCPU>
			struct PrecisionInfo<Resolution_1ms, FCPU> {
				static const TimerPrescalar Prescalar = TimerPrescalar::Prescalar_64;
				enum {
					OcrValue = FCPU / TimerPrescalarValue<Prescalar>::value / 1000UL,
					usDivider = 1000
				};
				VOID_STATIC_ASSERT(OcrValue <= 255);
			};
			template <uint32_t FCPU>
			struct PrecisionInfo<Resolution_100us, FCPU> {
				static const TimerPrescalar Prescalar = TimerPrescalar::Prescalar_8;

				enum {
					OcrValue = FCPU / TimerPrescalarValue<Prescalar>::value / 10000UL,
					usDivider = 100
				};
				VOID_STATIC_ASSERT(OcrValue <= 255);
			};
			template <uint32_t FCPU>
			struct PrecisionInfo<Resolution_10us, FCPU> {
				static const TimerPrescalar Prescalar = TimerPrescalar::Prescalar_1;

				enum {
					OcrValue = FCPU / TimerPrescalarValue<Prescalar>::value / 100000UL,
					usDivider = 10
				};
				VOID_STATIC_ASSERT(OcrValue <= 255);
			};
		}
//================================TIMER1=============================================================
		namespace timer1 {
			template <TimerPrescalar P>
			struct Timer1PrescalarData;
			DECLARE_TIMER_PRESCALAR(Timer1PrescalarData, Stopped, 0);
			DECLARE_TIMER_PRESCALAR(Timer1PrescalarData, Prescalar_1, 1 << CS10);
			DECLARE_TIMER_PRESCALAR(Timer1PrescalarData, Prescalar_8, 1 << CS11);
			DECLARE_TIMER_PRESCALAR(Timer1PrescalarData, Prescalar_64, (1 << CS11) | (1 << CS10) );
			DECLARE_TIMER_PRESCALAR(Timer1PrescalarData, Prescalar_256, 1 << CS12);
			DECLARE_TIMER_PRESCALAR(Timer1PrescalarData, Prescalar_1024, (1 << CS12) | (1 << CS10) );
			template <TimerPrecision R, uint32_t FCPU>
			struct Precision1Info;


			struct THwTimer1 {
					IO_REG_WRAPPER(TCCR1A, Tccra, uint8_t);
					IO_REG_WRAPPER(TCCR1B, Tccrb, uint8_t);
					IO_REG_WRAPPER(TCCR1C, Tccrc, uint8_t);
					IO_REG_WRAPPER(OCR1AL, Ocral, uint8_t);
					IO_REG_WRAPPER(OCR1AH, Ocrah, uint8_t);
					IO_REG_WRAPPER(OCR1BL, Ocrbl, uint8_t);
					IO_REG_WRAPPER(OCR1BH, Ocrbh, uint8_t);
					IO_REG_WRAPPER(TIMSK1, Timsk, uint8_t);
					IO_REG_WRAPPER(TIFR1, Tifr1, uint8_t);
					IO_REG_WRAPPER(TCNT1H, Tcnth, uint8_t);
					IO_REG_WRAPPER(TCNT1L, Tcntl, uint8_t);
					static inline uint8_t _prescalarBits = Timer1PrescalarData<Stopped>::Bits;
					typedef void(*timerTickCallback_t)();
				private:
					static inline volatile timerTickCallback_t _cbOCRA_COMP = nullptr;

				public:

					static void IntOCRA_Handler() {
						if (_cbOCRA_COMP != nullptr) { _cbOCRA_COMP(); }
					}

					typedef u16 counter_t;
					enum {
						maxCounterValue = 65535
					};

					enum PwmMode {
						DisabledPwm,
						DirectPwm, //������ ���(����� ��� ���������� � ��������� ��� ��������� �����)
						InversePwm //�������� ��� (����� ��� ��������� � ��������� ��� ����������)
					};


					enum TimerMode {
						Normal = 0,
						FastPwm = (1 << WGM10) | ((1 << WGM12) << 8),
						//PwmOCR1A = (1 << WGM10) | (1 << WGM11) | ((1 << WGM12) << 8) | ((1 << WGM13) << 8),
						Ctc = (1 << WGM12) << 8,
					};


					template < TimerPrecision P>
					static uint16_t tickPeriod_us() {
						return Precision1Info<P, 0>::usDivider;
					}

					static void reset() {
						Tcnth::Set(0);
						Tcntl::Set(0);
					}

					static void setMode(TimerMode mode) {
						/*u16 currentMode = Tccra::Get() | (Tccrb::Get() << 8);
						currentMode &= ~((1 << WGM10) | (1 << WGM11) | ((1 << WGM12) << 8) | ((1 << WGM13) << 8));
						currentMode |= mode;
						Tccra::Set(vd::lowByte(currentMode));
						Tccrb::Set(vd::hiByte(currentMode));*/
						const u16 mask = (u16)(~((1 << WGM10) | (1 << WGM11) | ((1 << WGM12) << 8) | ((1 << WGM13) << 8)));
						Tccra::AndOr(vd::lowByte(mask), vd::lowByte(mode));
						Tccrb::AndOr(vd::hiByte(mask), vd::hiByte(mode));
					}

					template <TimerPrescalar P>
					static void setPrescalar() {
						Tccrb::AndOr((uint8_t)(~((1 << CS10) | (1 << CS11) | (1 << CS12))), Timer1PrescalarData<P>::Bits);
					}
					static void setPrescalar(uint8_t bits) {
						Tccrb::AndOr((uint8_t)(~((1 << CS10) | (1 << CS11) | (1 << CS12))), bits);
					}

					template <PwmChannel channel, PwmMode mode, TimerPrescalar prescalar = TimerPrescalar::Prescalar_1, uint32_t FCPU>
					static void initPwm(uint8_t value) {
						setMode(TimerMode::FastPwm);
						_prescalarBits = Timer1PrescalarData<prescalar>::Bits;

						if (channel == Channel_0) {
							const u08 Mask = (u08)(~((1 << COM1A1) | (1 << COM1A0)));

							if (mode == DirectPwm) {
								Tccra::AndOr(Mask, (uint8_t)((1 << COM1A1) | (0 << COM1A0)));
							} else if (mode == InversePwm) { //InversePwm
								Tccra::AndOr(Mask, (uint8_t)((1 << COM1A1) | (1 << COM1A0)));
							} else if (mode == DisabledPwm) {
								Tccra::AndOr(Mask, (uint8_t)((0 << COM1A1) | (0 << COM1A0)));
							}

							cph::io::Pb1::SetConfiguration(cph::io::Pb1::Port::Out); //OC1A
						} else if (channel == Channel_1) {
							const u08 Mask = (u08)(~((1 << COM1B1) | (1 << COM1B0)));

							if (mode == DirectPwm) {
								Tccra::AndOr (Mask, (uint8_t)((1 << COM1B1) | (0 << COM1B0)));
							} else if (mode == InversePwm) { //InversePwm
								Tccra::AndOr(Mask, (uint8_t)((1 << COM1B1) | (1 << COM1B0)));
							} else if (mode == DisabledPwm) {
								Tccra::AndOr(Mask, (uint8_t)((0 << COM1B1) | (0 << COM1B0)));
							}

							cph::io::Pb2::SetConfiguration(cph::io::Pb2::Port::Out); //OC1B
						}

						pwmSetValuePercent<channel>(value);
						Tcnth::Set(0);
						Tcntl::Set(0);
						setPrescalar<prescalar>();
					}

					static inline bool _isInverted = false;
					template <PwmChannel channel, uint32_t Period, uint32_t FCPU>
					static void initPwmPeriod(bool start, bool isInverted = false) {
						VOID_STATIC_ASSERT_MSG(channel ==
						                       Channel_0, "Channel_1_not_supported_in_AVR");
						_isInverted = isInverted;
						setMode(TimerMode::Ctc);
						typedef PwmFreqCalculator < Period, uint16_t, FCPU> FREQ_CALC;
						const TimerPrescalar prescalar = cph::TimerPrescalar::Prescalar_1;//FREQ_CALC::currentPrescalar;
						const uint16_t ocrValue = FREQ_CALC::ocrValue;
						_prescalarBits = Timer1PrescalarData<prescalar>::Bits;
						//if (channel == Channel_0) {
						const u08 Mask = (u08)(~((1 << COM1A1) | (1 << COM1A0)));
						Tccra::AndOr(Mask, (uint8_t)((0 << COM1A1) | (1 <<
						                                    COM1A0)));//������������ OC0 ��� Compare Match
						Ocrah::Set(vd::hiByte(ocrValue));
						Ocral::Set(vd::lowByte(ocrValue));
						cph::io::Pb1::SetConfiguration(cph::io::Pb1::Port::Out); //OC1A
						/*} else if (channel == Channel_1) {
							const u08 Mask = ~((1 << COM1B1) | (1 << COM1B0));
							Tccra::AndOr(Mask, (0 << COM1B1) | (1 <<
							                                    COM1B0));//������������ OC1 ��� Compare Match
							Ocrbh::Set(vd::hiByte(ocrValue));
							Ocrbl::Set(vd::lowByte(ocrValue));
						}*/
						//
						Tcnth::Set(0);
						Tcntl::Set(0);

						if (start) {
							setPrescalar<prescalar>();
						} else {
							setPrescalar<Stopped>();
						}
					}

					template <PwmChannel channel>
					static void disablePwmPeriod() {
						setPrescalar<Stopped>();
						VOID_STATIC_ASSERT_MSG(channel ==
						                       Channel_0, "Channel_1_not_supported_in_AVR");
						const u08 Mask = (u08)(~((1 << COM1A1) | (1 << COM1A0)));
						Tccra::AndOr(Mask, (uint8_t)((0 << COM1A1) | (0 <<
						                                    COM1A0)));//������������ OC0 ��� Compare Match
						cph::io::Pb1::Set(_isInverted);
					}

					template <PwmChannel channel>
					static void enablePwmPeriod() {
						VOID_STATIC_ASSERT_MSG(channel ==
						                       Channel_0, "Channel_1_not_supported_in_AVR");
						const u08 Mask = (u08)(~((1 << COM1A1) | (1 << COM1A0)));
						Tccra::AndOr(Mask, (uint8_t)((0 << COM1A1) | (1 <<
						                                    COM1A0)));//������������ OC0 ��� Compare Match
						Tcnth::Set(0);
						Tcntl::Set(0);
						setPrescalar(_prescalarBits);
					}

					template <PwmChannel channel>
					static void enablePwm() {
						Tcnth::Set(0);
						Tcntl::Set(0);
						setPrescalar(_prescalarBits);
					}

					static void disablePwm() {
						setPrescalar<Stopped>();
					}

					template <PwmChannel channel>
					static void pwmSetValuePercent(uint8_t value) {
						uint16_t out = (uint16_t)((255 * value) / 100);
						ATOMIC_BLOCK() {
							if (channel == Channel_0) {
								Ocrah::Set(vd::hiByte(out));
								Ocral::Set(vd::lowByte(out));
							} else if (channel == Channel_1) {
								Ocrbh::Set(vd::hiByte(out));
								Ocrbl::Set(vd::lowByte(out));
							}
						}
					}

					template <TimerPrecision R, uint32_t FCPU>
					static void initPrecisionTimer(timerTickCallback_t cbTick, bool IsEnable) {
						Ocrah::Set(vd::hiByte(Precision1Info<R, FCPU>::OcrValue));
						Ocral::Set(vd::lowByte(Precision1Info<R, FCPU>::OcrValue));
						setMode(TimerMode::Ctc);
						_cbOCRA_COMP = cbTick;
						_prescalarBits = Timer1PrescalarData<Precision1Info<R, FCPU>::Prescalar>::Bits;
						if (IsEnable) { setPrescalar<Precision1Info<R, FCPU>::Prescalar>(); } else { setPrescalar<Stopped>(); }
					}
					/*
					template <TimerPrescalar P, class Period, uint32_t time, uint32_t FCPU>
					static void initCustomTimer(timerTickCallback_t cbTick,bool IsEnable) {
						typedef TimerPrescalarValue<P> prescalar;
						//const uint32_t ocrValue = (usTickTime * FCPU) / 1000000 / (prescalar::value);
						const uint32_t ocrValue = ((FCPU) / (1000UL * Period::Denom) * Period::Num * time) /
						                          (prescalar::value);
						VOID_STATIC_ASSERT(ocrValue <= maxCounterValue); //make Prescalar bigger!
						Ocrah::Set(vd::hiByte(ocrValue));
						Ocral::Set(vd::lowByte(ocrValue));
						setMode(TimerMode::Ctc);
						_cbOCRA_COMP = cbTick;
						_prescalarBits = Timer1PrescalarData<P>::Bits;
						if (IsEnable) { setPrescalar<P>(); } else { setPrescalar<Stopped>(); }
					}
					*/


					template <uint32_t FCPU, class Period>
					constexpr static void initCustomTimer(const vd::chrono::any_t<Period>& tickTime,
					                                      timerTickCallback_t cbTick, bool IsEnable) {
						auto[prescalar,
						     ocrValue] = cph::Private::timer::calculatePrescalar <  maxCounterValue, FCPU, Period>(tickTime);
						assert(ocrValue <= maxCounterValue); //make Prescalar bigger!
						Ocrah::Set((uint8_t)vd::hiByte(ocrValue));
						Ocral::Set((uint8_t)vd::lowByte(ocrValue));
						setMode(TimerMode::Ctc);
						_cbOCRA_COMP = cbTick;
						_prescalarBits = (uint8_t) getTimerPrescalarBits< Timer1PrescalarData>(prescalar);
						if (IsEnable) { setPrescalar(_prescalarBits); } else { setPrescalar<Stopped>(); }

						/*constexpr TimerPrescalar FindedPrescalar
							= (TimerPrescalar)(::cph::Private::PreselectorIterator<PrescalarValue, true, 0>::value);
						//TimerPrescalar P,
						typedef TimerPrescalarValue<P> prescalar;
						//const uint32_t ocrValue = (tickTime * FCPU / 1000000) / (prescalar::value);
						const uint32_t ocrValue = ((FCPU) / (1000UL * Period::Denom) * Period::Num * tickTime.count()) /
						                          (prescalar::value);
						assert(ocrValue <= maxCounterValue); //make Prescalar bigger!
						Ocrah::Set(vd::hiByte(ocrValue));
						Ocral::Set(vd::lowByte(ocrValue));
						_cbOCRA_COMP = cbTick;
						setMode(TimerMode::Ctc);
						_prescalarBits = Timer1PrescalarData<P>::Bits;
						if (IsEnable) { setPrescalar<P>(); } else { setPrescalar<Stopped>(); }*/
					}


					static void enableTimer() {
						Tcnth::Set(0);
						Tcntl::Set(0);
						Timsk::Or(1 << OCIE1A);
						setPrescalar(_prescalarBits);
					}
					static void disableTimer() {
						setPrescalar<Stopped>();
						Timsk::AndNot(1 << OCIE1A);
					}
			};

			template <uint32_t FCPU>
			struct Precision1Info<Resolution_1ms, FCPU> {
				static const TimerPrescalar Prescalar = TimerPrescalar::Prescalar_64;
				enum {
					OcrValue = FCPU / TimerPrescalarValue<Prescalar>::value / 1000UL,
					usDivider = 1000
				};
				VOID_STATIC_ASSERT((uint32_t)OcrValue <= THwTimer1::maxCounterValue);
			};
			template <uint32_t FCPU>
			struct Precision1Info<Resolution_100us, FCPU> {
				static const TimerPrescalar Prescalar = TimerPrescalar::Prescalar_8;

				enum {
					OcrValue = FCPU / TimerPrescalarValue<Prescalar>::value / 10000UL,
					usDivider = 100
				};
				VOID_STATIC_ASSERT((uint32_t)OcrValue <= THwTimer1::maxCounterValue);
			};
			template <uint32_t FCPU>
			struct Precision1Info<Resolution_10us, FCPU> {
				static const TimerPrescalar Prescalar = TimerPrescalar::Prescalar_1;

				enum {
					OcrValue = FCPU / TimerPrescalarValue<Prescalar>::value / 100000UL,
					usDivider = 10
				};
				VOID_STATIC_ASSERT((uint32_t)OcrValue <= THwTimer1::maxCounterValue);
			};
		}


	}
}

#elif defined(__AVR_ATmega16__)

#define CPH_HAS_TIMER0
#define CPH_INT_TIMER0_OCRA TIMER0_COMP_vect
#define CPH_HAS_TIMER1
#define CPH_INT_TIMER1_OCRA TIMER1_COMPA_vect
namespace cph {
	namespace Private {
		namespace timer0 {

			template <TimerPrescalar P>
			struct Timer0PrescalarData;
			DECLARE_TIMER_PRESCALAR(Timer0PrescalarData, Stopped, 0);
			DECLARE_TIMER_PRESCALAR(Timer0PrescalarData, Prescalar_1, 1 << CS00);
			DECLARE_TIMER_PRESCALAR(Timer0PrescalarData, Prescalar_8, 1 << CS01);
			DECLARE_TIMER_PRESCALAR(Timer0PrescalarData, Prescalar_64, (1 << CS00) | (1 << CS01));
			DECLARE_TIMER_PRESCALAR(Timer0PrescalarData, Prescalar_256, 1 << CS02);
			DECLARE_TIMER_PRESCALAR(Timer0PrescalarData, Prescalar_1024, (1 << CS00) | (1 << CS02));

			template <TimerPrecision R, uint32_t FCPU>
			struct Precision0Info;


			struct THwTimer0 {
					IO_REG_WRAPPER(TCCR0, Tccra, uint8_t);
					IO_REG_WRAPPER(OCR0, Ocra, uint8_t);
					IO_REG_WRAPPER(TIMSK, Timsk, uint8_t);
					IO_REG_WRAPPER(TCNT0, Tcnt, uint8_t);
					static inline uint8_t _prescalarBits = Timer0PrescalarData<Stopped>::Bits;
					typedef u08 counter_t;
					enum {
						maxCounterValue = 255
					};
					typedef void(*timerTickCallback_t)();
				private:
					static inline volatile timerTickCallback_t _cbOCRA_COMP = nullptr;

				public:
					static void IntOCRA_Handler() {
						if (_cbOCRA_COMP != nullptr) { _cbOCRA_COMP(); }
					}
					enum TimerMode {
						Normal = 0,
						FastPwm = (1 << WGM00) | (1 << WGM01),
						Ctc = 1 << WGM01
					};

					template <TimerPrecision P>
					static uint16_t tickPeriod_us() {
						return Precision0Info<P, 0>::usDivider;
					}

					static void reset() {
						Tcnt::Set(0);
					}

					static void setMode(TimerMode mode) {
						Tccra::AndOr(~((1 << WGM00) | (1 << WGM01)), mode);
					}

					template <TimerPrescalar P>
					static void setPrescalar() {
						typedef Timer0PrescalarData<P> info;
						Tccra::AndOr(~((1 << CS00) | (1 << CS01) | (1 << CS02)), info::Bits);
					}
					static void setPrescalar(uint8_t bits) {
						Tccra::AndOr(~((1 << CS00) | (1 << CS01) | (1 << CS02)), bits);
					}

					template <TimerPrecision R, uint32_t FCPU>
					static void initPrecisionTimer(timerTickCallback_t cbTick, bool IsEnable) {
						Ocra::Set(Precision0Info<R, FCPU>::OcrValue);
						setMode(TimerMode::Ctc);
						_cbOCRA_COMP = cbTick;
						_prescalarBits = Timer0PrescalarData<Precision0Info<R, FCPU>::Prescalar>::Bits;
						if (IsEnable) { setPrescalar<Precision0Info<R, FCPU>::Prescalar>(); } else { setPrescalar<Stopped>(); }
					}

					/*
					template <TimerPrescalar P, class Period, uint32_t time, uint32_t FCPU>
					static void initCustomTimer(timerTickCallback_t cbTick,bool IsEnable) {
						typedef TimerPrescalarValue<P> prescalar;
						//const uint32_t ocrValue = (usTickTime * FCPU) / 1000000 / (prescalar::value);
						const uint32_t ocrValue = ((FCPU) / (1000UL * Period::Denom) * Period::Num * time) /
						                          (prescalar::value);
						VOID_STATIC_ASSERT(ocrValue <= maxCounterValue); //make Prescalar bigger!
						Ocra::Set(vd::lowByte(ocrValue));
						_cbOCRA_COMP = cbTick;
						setMode(TimerMode::Ctc);
						_prescalarBits = Timer0PrescalarData<P>::Bits;
						if (IsEnable) { setPrescalar<P>(); } else { setPrescalar<Stopped>(); }
					}
					*/
					template <uint32_t FCPU, class Period>
					constexpr static void initCustomTimer(vd::chrono::any_t<Period> tickTime,
					                                      timerTickCallback_t cbTick, bool IsEnable) {
						/*typedef TimerPrescalarValue<P> prescalar;
						//const uint32_t ocrValue = (tickTime * FCPU / 1000000) / (prescalar::value);
						const uint32_t ocrValue = ((FCPU) / (1000UL * Period::Denom) * Period::Num * tickTime.count()) /
						                          (prescalar::value);*/
						auto[prescalar,
						     ocrValue] = cph::Private::timer::calculatePrescalar <  maxCounterValue, FCPU, Period>(tickTime);
						assert(ocrValue <= maxCounterValue); //make Prescalar bigger!
						Ocra::Set(vd::lowByte(ocrValue));
						_cbOCRA_COMP = cbTick;
						setMode(TimerMode::Ctc);
						_prescalarBits = getTimerPrescalarBits< Timer0PrescalarData>(prescalar);
						if (IsEnable) { setPrescalar(_prescalarBits); } else { setPrescalar<Stopped>(); }
					}

					static void enableTimer() {
						Tcnt::Set(0);
						Timsk::Or(1 << OCIE0);
						setPrescalar(_prescalarBits);
					}

					static void disableTimer() {
						setPrescalar<Stopped>();
						Timsk::AndNot(1 << OCIE0);
					}

			};


			template <uint32_t FCPU>
			struct Precision0Info<Resolution_1ms, FCPU> {
				static const TimerPrescalar Prescalar = TimerPrescalar::Prescalar_64;
				enum {
					OcrValue = FCPU / TimerPrescalarValue<Prescalar>::value / 1000UL,
					usDivider = 1000UL
				};
				VOID_STATIC_ASSERT(OcrValue <= 255);
			};
			template <uint32_t FCPU>
			struct Precision0Info<Resolution_100us, FCPU> {
				static const TimerPrescalar Prescalar = TimerPrescalar::Prescalar_8;

				enum {
					OcrValue = FCPU / TimerPrescalarValue<Prescalar>::value / 10000UL,
					usDivider = 100UL
				};
				VOID_STATIC_ASSERT(OcrValue <= 255);
			};
			template <uint32_t FCPU>
			struct Precision0Info<Resolution_10us, FCPU> {
				static const TimerPrescalar Prescalar = TimerPrescalar::Prescalar_1;

				enum {
					OcrValue = FCPU / TimerPrescalarValue<Prescalar>::value / 100000UL,
					usDivider = 10UL
				};
				VOID_STATIC_ASSERT(OcrValue <= 255);
			};
		}
//================================TIMER1=============================================================
		namespace timer1 {
			template <TimerPrescalar P>
			struct Timer1PrescalarData;

			DECLARE_TIMER_PRESCALAR(Timer1PrescalarData, Stopped, 0);
			DECLARE_TIMER_PRESCALAR(Timer1PrescalarData, Prescalar_1, 1 << CS10);
			DECLARE_TIMER_PRESCALAR(Timer1PrescalarData, Prescalar_8, 1 << CS11);
			DECLARE_TIMER_PRESCALAR(Timer1PrescalarData, Prescalar_64, (1 << CS10) | (1 << CS11));
			DECLARE_TIMER_PRESCALAR(Timer1PrescalarData, Prescalar_256, 1 << CS12);
			DECLARE_TIMER_PRESCALAR(Timer1PrescalarData, Prescalar_1024, (1 << CS10) | (1 << CS12));


			template <TimerPrecision R, uint32_t FCPU>
			struct Precision1Info;


			struct THwTimer1 {
					IO_REG_WRAPPER(TCCR1A, Tccra, uint8_t);
					IO_REG_WRAPPER(TCCR1B, Tccrb, uint8_t);
					IO_REG_WRAPPER(OCR1AL, Ocral, uint8_t);
					IO_REG_WRAPPER(OCR1AH, Ocrah, uint8_t);
					IO_REG_WRAPPER(OCR1BL, Ocrbl, uint8_t);
					IO_REG_WRAPPER(OCR1BH, Ocrbh, uint8_t);
					IO_REG_WRAPPER(TIMSK, Timsk, uint8_t);
					IO_REG_WRAPPER(TCNT1H, Tcnth, uint8_t);
					IO_REG_WRAPPER(TCNT1L, Tcntl, uint8_t);
					static inline uint8_t _prescalarBits = Timer1PrescalarData<Stopped>::Bits;
					enum {
						maxCounterValue = 65535
					};

					typedef void(*timerTickCallback_t)();
				private:
					static inline volatile timerTickCallback_t _cbOCRA_COMP = nullptr;

				public:
					static void IntOCRA_Handler() {
						if (_cbOCRA_COMP != nullptr) { _cbOCRA_COMP(); }
					}
					typedef u16 counter_t;
					enum TimerMode {
						Normal = 0,
						FastPwm = (1 << WGM10) | ((1 << WGM12) << 8),
						Ctc = (1 << WGM12) << 8,
					};


					template <TimerPrecision P>
					static uint16_t tickPeriod_us() {
						return Precision1Info<P, 0>::usDivider;
					}

					static void reset() {
						Tcnth::Set(0);
						Tcntl::Set(0);
					}

					static void setMode(TimerMode mode) {
						u16 currentMode = Tccra::Get() | (Tccrb::Get() << 8);
						currentMode &= ~((1 << WGM10) | (1 << WGM11) | ((1 << WGM12) << 8) | ((1 << WGM13) << 8));
						currentMode |= mode;
						Tccra::Set(vd::lowByte(currentMode));
						Tccrb::Set(vd::hiByte(currentMode));
					}

					template <TimerPrescalar P>
					static void setPrescalar() {
						u08 currentP = Tccrb::Get() & (~((1 << CS10) | (1 << CS11) | (1 << CS12)));
						Tccrb::AndOr(currentP, Timer1PrescalarData<P>::Bits);
					}
					static void setPrescalar(uint8_t bits) {
						Tccrb::AndOr(~((1 << CS10) | (1 << CS11) | (1 << CS12)), bits);
					}


					template <TimerPrecision R, uint32_t FCPU>
					static void initPrecisionTimer(timerTickCallback_t cbTick, bool IsEnable) {
						Ocrah::Set(vd::hiByte(Precision1Info<R, FCPU>::OcrValue));
						Ocral::Set(vd::lowByte(Precision1Info<R, FCPU>::OcrValue));
						setMode(TimerMode::Ctc);
						_cbOCRA_COMP = cbTick;
						_prescalarBits = Timer1PrescalarData<Precision1Info<R, FCPU>::Prescalar>::Bits;
						if (IsEnable) { setPrescalar<Precision1Info<R, FCPU>::Prescalar>(); } else { setPrescalar<Stopped>(); }
					}
					/*
					template <TimerPrescalar P, class Period, uint32_t time, uint32_t FCPU>
					static void initCustomTimer(timerTickCallback_t cbTick,bool IsEnable) {
						typedef TimerPrescalarValue<P> prescalar;
						//const uint32_t ocrValue = (usTickTime * FCPU) / 1000000 / (prescalar::value);
						const uint32_t ocrValue = ((FCPU) / (1000UL * Period::Denom) * Period::Num * time) /
						                          (prescalar::value);
						VOID_STATIC_ASSERT(ocrValue <= maxCounterValue); //make Prescalar bigger!
						Ocrah::Set(vd::hiByte(ocrValue));
						Ocral::Set(vd::lowByte(ocrValue));
						_cbOCRA_COMP = cbTick;
						setMode(TimerMode::Ctc);
						_prescalarBits = Timer1PrescalarData<P>::Bits;
						if (IsEnable) { setPrescalar<P>(); } else { setPrescalar<Stopped>(); }
					}
					*/
					template <uint32_t FCPU, class Period>
					constexpr static void initCustomTimer(const vd::chrono::any_t<Period>& tickTime,
					                                      timerTickCallback_t cbTick, bool IsEnable) {
						auto[prescalar,
						     ocrValue] = cph::Private::timer::calculatePrescalar <  maxCounterValue, FCPU, Period>(tickTime);
						assert(ocrValue <= maxCounterValue); //make Prescalar bigger!
						Ocrah::Set(vd::hiByte(ocrValue));
						Ocral::Set(vd::lowByte(ocrValue));
						_cbOCRA_COMP = cbTick;
						setMode(TimerMode::Ctc);
						_prescalarBits = getTimerPrescalarBits< Timer1PrescalarData>(prescalar);
						if (IsEnable) { setPrescalar(_prescalarBits); } else { setPrescalar<Stopped>(); }
					}

					static void enableTimer() {
						Tcnth::Set(0);
						Tcntl::Set(0);
						Timsk::Or(1 << OCIE1A);
						setPrescalar(_prescalarBits);
					}

					static void disableTimer() {
						setPrescalar<Stopped>();
						Timsk::AndNot(1 << OCIE1A);
					}

			};

			template <uint32_t FCPU>
			struct Precision1Info<Resolution_1ms, FCPU> {
				static const TimerPrescalar Prescalar = TimerPrescalar::Prescalar_64;
				enum {
					OcrValue = FCPU / TimerPrescalarValue<Prescalar>::value / 1000UL,
					usDivider = 1000UL
				};
				VOID_STATIC_ASSERT((uint32_t)OcrValue <= THwTimer1::maxCounterValue);
			};
			template <uint32_t FCPU>
			struct Precision1Info<Resolution_100us, FCPU> {
				static const TimerPrescalar Prescalar = TimerPrescalar::Prescalar_8;

				enum {
					OcrValue = FCPU / TimerPrescalarValue<Prescalar>::value / 10000UL,
					usDivider = 100UL
				};
				VOID_STATIC_ASSERT((uint32_t)OcrValue <= THwTimer1::maxCounterValue);
			};
			template <uint32_t FCPU>
			struct Precision1Info<Resolution_10us, FCPU> {
				static const TimerPrescalar Prescalar = TimerPrescalar::Prescalar_1;

				enum {
					OcrValue = FCPU / TimerPrescalarValue<Prescalar>::value / 100000UL,
					usDivider = 10UL
				};
				VOID_STATIC_ASSERT((uint32_t)OcrValue <= THwTimer1::maxCounterValue);
			};
		}


	}
}
#elif defined(__AVR_ATmega16__) || defined (__AVR_ATmega8__)


#ifdef __AVR_ATmega16__
	#define CPH_HAS_TIMER0
	#define CPH_INT_TIMER0_OCRA TIMER0_COMP_vect
#endif
#define CPH_HAS_TIMER1
#define CPH_INT_TIMER1_OCRA TIMER1_COMPA_vect
namespace cph {
	namespace Private {
		#ifdef CPH_HAS_TIMER0
		namespace timer0 {
			template <TimerPrescalar P>
			struct Timer0PrescalarData;

			DECLARE_TIMER_PRESCALAR(Timer0PrescalarData, Stopped, 0);
			DECLARE_TIMER_PRESCALAR(Timer0PrescalarData, Prescalar_1, 1 << CS00);
			DECLARE_TIMER_PRESCALAR(Timer0PrescalarData, Prescalar_8, 1 << CS01);
			DECLARE_TIMER_PRESCALAR(Timer0PrescalarData, Prescalar_64, (1 << CS00) | (1 << CS01));
			DECLARE_TIMER_PRESCALAR(Timer0PrescalarData, Prescalar_256, 1 << CS02);
			DECLARE_TIMER_PRESCALAR(Timer0PrescalarData, Prescalar_1024, (1 << CS00) | (1 << CS02));

			template <TimerPrecision R, uint32_t FCPU>
			struct PrecisionInfo;


			struct THwTimer0 {
					IO_REG_WRAPPER(TCCR0, Tccra, uint8_t);
					IO_REG_WRAPPER(OCR0, Ocra, uint8_t);
					IO_REG_WRAPPER(TIMSK, Timsk, uint8_t);
					IO_REG_WRAPPER(TCNT0, Tcnt, uint8_t);
					static inline uint8_t _prescalarBits = 0;
					typedef u08 counter_t;
					enum {
						maxCounterValue = 255
					};
					typedef void(*timerTickCallback_t)();
				private:
					static inline volatile timerTickCallback_t _cbOCRA_COMP = nullptr;

				public:
					static void IntOCRA_Handler() {
						if (_cbOCRA_COMP != nullptr) { _cbOCRA_COMP(); }
					}
					enum TimerMode {
						Normal = 0,
						FastPwm = (1 << WGM00) | (1 << WGM01),
						Ctc = 1 << WGM01
					};

					template <TimerPrecision P>
					static uint16_t tickPeriod_us() {
						return PrecisionInfo<P, 0>::usDivider;
					}

					static void reset() {
						Tcnt::Set(0);
					}

					static void setMode(TimerMode mode) {
						Tccra::AndOr(~((1 << WGM00) | (1 << WGM01)), mode);
					}

					template <TimerPrescalar P>
					static void setPrescalar() {
						typedef Timer0PrescalarData<P> info;
						Tccra::AndOr(~((1 << CS00) | (1 << CS01) | (1 << CS02)), info::Bits);
					}
					static void setPrescalar(uint8_t bits) {
						Tccra::AndOr(~((1 << CS00) | (1 << CS01) | (1 << CS02)), bits);
					}

					template <TimerPrecision R, uint32_t FCPU>
					constexpr static void initPrecisionTimer(timerTickCallback_t cbTick, bool IsEnable) {
						Ocra::Set(PrecisionInfo<R, FCPU>::OcrValue);
						setMode(TimerMode::Ctc);
						_cbOCRA_COMP = cbTick;
						_prescalarBits = Timer0PrescalarData<PrecisionInfo<R, FCPU>::Prescalar>::Bits;
						if (IsEnable) { setPrescalar<PrecisionInfo<R, FCPU>::Prescalar>(); } else { setPrescalar<Stopped>(); }
					}

					/*
					template <TimerPrescalar P, class Period, uint32_t time, uint32_t FCPU>
					static void initCustomTimer(timerTickCallback_t cbTick,bool IsEnable) {
						typedef TimerPrescalarValue<P> prescalar;
						//const uint32_t ocrValue = (usTickTime * FCPU) / 1000000 / (prescalar::value);
						const uint32_t ocrValue = ((FCPU) / (1000UL * Period::Denom) * Period::Num * time) /
						                          (prescalar::value);
						VOID_STATIC_ASSERT(ocrValue <= maxCounterValue); //make Prescalar bigger!
						Ocra::Set(vd::lowByte(ocrValue));
						_cbOCRA_COMP = cbTick;
						setMode(TimerMode::Ctc);
						_prescalarBits = Timer0PrescalarData<P>::Bits;
						if (IsEnable) { setPrescalar<P>(); } else { setPrescalar<Stopped>(); }
					}
					*/
					template <uint32_t FCPU, class Period>
					constexpr static void initCustomTimer(const vd::chrono::any_t<Period>& tickTime,
					                                      timerTickCallback_t cbTick, bool IsEnable) {
						auto[prescalar,
						     ocrValue] = cph::Private::timer::calculatePrescalar <  maxCounterValue, FCPU, Period>(tickTime);
						assert(ocrValue <= maxCounterValue); //make Prescalar bigger!
						Ocra::Set(vd::lowByte(ocrValue));
						_cbOCRA_COMP = cbTick;
						setMode(TimerMode::Ctc);
						_prescalarBits = getTimerPrescalarBits< Timer0PrescalarData>(prescalar);
						if (IsEnable) { setPrescalar(_prescalarBits); } else { setPrescalar<Stopped>(); }
					}

					static void enableTimer() {
						Tcnt::Set(0);
						Timsk::Or(1 << OCIE0);
						setPrescalar(_prescalarBits);
					}

					static void disableTimer() {
						setPrescalar<Stopped>();
						Timsk::AndNot(1 << OCIE0);
					}

			};


			template <uint32_t FCPU>
			struct PrecisionInfo<Resolution_1ms, FCPU> {
				static const TimerPrescalar Prescalar = TimerPrescalar::Prescalar_64;
				enum {
					OcrValue = FCPU / TimerPrescalarValue<Prescalar>::value / 1000UL,
					usDivider = 1000UL
				};
				VOID_STATIC_ASSERT(OcrValue <= 255);
			};
			template <uint32_t FCPU>
			struct PrecisionInfo<Resolution_100us, FCPU> {
				static const TimerPrescalar Prescalar = TimerPrescalar::Prescalar_8;

				enum {
					OcrValue = FCPU / TimerPrescalarValue<Prescalar>::value / 10000UL,
					usDivider = 100UL
				};
				VOID_STATIC_ASSERT(OcrValue <= 255);
			};
			template <uint32_t FCPU>
			struct PrecisionInfo<Resolution_10us, FCPU> {
				static const TimerPrescalar Prescalar = TimerPrescalar::Prescalar_1;

				enum {
					OcrValue = FCPU / TimerPrescalarValue<Prescalar>::value / 100000UL,
					usDivider = 10UL
				};
				VOID_STATIC_ASSERT(OcrValue <= 255);
			};
		}
		#endif
		//================================TIMER1=============================================================
		#ifdef CPH_HAS_TIMER1
		namespace timer1 {
			template <TimerPrescalar P>
			struct Timer1PrescalarData;
			DECLARE_TIMER_PRESCALAR(Timer1PrescalarData, Stopped, 0);
			DECLARE_TIMER_PRESCALAR(Timer1PrescalarData, Prescalar_1, 1 << CS10);
			DECLARE_TIMER_PRESCALAR(Timer1PrescalarData, Prescalar_8, 1 << CS11);
			DECLARE_TIMER_PRESCALAR(Timer1PrescalarData, Prescalar_64, (1 << CS10) | (1 << CS11));
			DECLARE_TIMER_PRESCALAR(Timer1PrescalarData, Prescalar_256, 1 << CS12);
			DECLARE_TIMER_PRESCALAR(Timer1PrescalarData, Prescalar_1024, (1 << CS10) | (1 << CS12));


			template <TimerPrecision R, uint32_t FCPU>
			struct Precision1Info;


			struct THwTimer1 {
					IO_REG_WRAPPER(TCCR1A, Tccra, uint8_t);
					IO_REG_WRAPPER(TCCR1B, Tccrb, uint8_t);
					IO_REG_WRAPPER(OCR1AL, Ocral, uint8_t);
					IO_REG_WRAPPER(OCR1AH, Ocrah, uint8_t);
					IO_REG_WRAPPER(OCR1BL, Ocrbl, uint8_t);
					IO_REG_WRAPPER(OCR1BH, Ocrbh, uint8_t);
					IO_REG_WRAPPER(TIMSK, Timsk, uint8_t);
					IO_REG_WRAPPER(TCNT1H, Tcnth, uint8_t);
					IO_REG_WRAPPER(TCNT1L, Tcntl, uint8_t);
					static inline uint8_t _prescalarBits = 0;
					enum {
						maxCounterValue = 65535
					};

					typedef void(*timerTickCallback_t)();
				private:
					static volatile inline timerTickCallback_t _cbOCRA_COMP = nullptr;

				public:
					static void IntOCRA_Handler() {
						if (_cbOCRA_COMP != nullptr) { _cbOCRA_COMP(); }
					}
					typedef u16 counter_t;
					enum TimerMode {
						Normal = 0,
						FastPwm = ((1 << WGM12) << 8) | (1 << WGM10),
						Ctc = (1 << WGM12) << 8,
					};


					template <TimerPrecision P>
					static uint16_t tickPeriod_us() {
						return Precision1Info<P, 0>::usDivider;
					}

					static void reset() {
						Tcnth::Set(0);
						Tcntl::Set(0);
					}

					static void setMode(TimerMode mode) {
						u16 currentMode = Tccra::Get() | (Tccrb::Get() << 8);
						currentMode &= ~((1 << WGM10) | (1 << WGM11) | ((1 << WGM12) << 8) | ((1 << WGM13) << 8));
						currentMode |= mode;
						Tccra::Set(vd::lowByte(currentMode));
						Tccrb::Set(vd::hiByte(currentMode));
					}

					template <TimerPrescalar P>
					static void setPrescalar() {
						u08 currentP = Tccrb::Get() & (~((1 << CS10) | (1 << CS11) | (1 << CS12)));
						Tccrb::AndOr(currentP, Timer1PrescalarData<P>::Bits);
					}
					static void setPrescalar(uint8_t bits) {
						Tccrb::AndOr(~((1 << CS10) | (1 << CS11) | (1 << CS12)), bits);
					}


					template <TimerPrecision R, uint32_t FCPU>
					constexpr static void initPrecisionTimer(timerTickCallback_t cbTick, bool IsEnable) {
						Ocrah::Set(vd::hiByte(Precision1Info<R, FCPU>::OcrValue));
						Ocral::Set(vd::lowByte(Precision1Info<R, FCPU>::OcrValue));
						setMode(TimerMode::Ctc);
						_cbOCRA_COMP = cbTick;
						_prescalarBits = Timer1PrescalarData<Precision1Info<R, FCPU>::Prescalar>::Bits;
						if (IsEnable) { setPrescalar<Precision1Info<R, FCPU>::Prescalar>(); } else { setPrescalar<Stopped>(); }
					}
					/*
					template <TimerPrescalar P, class Period, uint32_t time, uint32_t FCPU>
					static void initCustomTimer(timerTickCallback_t cbTick,bool IsEnable) {
						typedef TimerPrescalarValue<P> prescalar;
						//const uint32_t ocrValue = (usTickTime * FCPU) / 1000000 / (prescalar::value);
						const uint32_t ocrValue = ((FCPU) / (1000UL * Period::Denom) * Period::Num * time) /
						                          (prescalar::value);
						VOID_STATIC_ASSERT(ocrValue <= maxCounterValue); //make Prescalar bigger!
						Ocrah::Set(vd::hiByte(ocrValue));
						Ocral::Set(vd::lowByte(ocrValue));
						_cbOCRA_COMP = cbTick;
						setMode(TimerMode::Ctc);
						_prescalarBits = Timer1PrescalarData<P>::Bits;
						if (IsEnable) { setPrescalar<P>(); } else { setPrescalar<Stopped>(); }
					}*/

					template <uint32_t FCPU, class Period>
					static void initCustomTimer(const vd::chrono::any_t<Period>& tickTime, timerTickCallback_t cbTick,
					                            bool IsEnable) {
						//typedef TimerPrescalarValue<P> prescalar;
						auto[prescalar,
						     ocrValue] = cph::Private::timer::calculatePrescalar <  maxCounterValue, FCPU, Period>(tickTime);
						assert(ocrValue <= maxCounterValue); //make Prescalar bigger!
						Ocrah::Set(vd::hiByte(ocrValue));
						Ocral::Set(vd::lowByte(ocrValue));
						_cbOCRA_COMP = cbTick;
						setMode(TimerMode::Ctc);
						_prescalarBits = getTimerPrescalarBits< Timer1PrescalarData>(prescalar);
						if (IsEnable) { setPrescalar(_prescalarBits); } else { setPrescalar<Stopped>(); }
					}

					static void enableTimer() {
						Tcnth::Set(0);
						Tcntl::Set(0);
						Timsk::Or(1 << OCIE1A);
						setPrescalar(_prescalarBits);
					}

					static void disableTimer() {
						setPrescalar<Stopped>();
						Timsk::AndNot(1 << OCIE1A);
					}

			};




			template <uint32_t FCPU>
			struct Precision1Info<Resolution_1ms, FCPU> {
				static const TimerPrescalar Prescalar = TimerPrescalar::Prescalar_64;
				enum {
					OcrValue = FCPU / TimerPrescalarValue<Prescalar>::value / 1000UL,
					usDivider = 1000UL
				};
				VOID_STATIC_ASSERT((uint32_t)OcrValue <= THwTimer1::maxCounterValue);
			};
			template <uint32_t FCPU>
			struct Precision1Info<Resolution_100us, FCPU> {
				static const TimerPrescalar Prescalar = TimerPrescalar::Prescalar_8;

				enum {
					OcrValue = FCPU / TimerPrescalarValue<Prescalar>::value / 10000UL,
					usDivider = 100UL
				};
				VOID_STATIC_ASSERT((uint32_t)OcrValue <= THwTimer1::maxCounterValue);
			};
			template <uint32_t FCPU>
			struct Precision1Info<Resolution_10us, FCPU> {
				static const TimerPrescalar Prescalar = TimerPrescalar::Prescalar_1;

				enum {
					OcrValue = FCPU / TimerPrescalarValue<Prescalar>::value / 100000UL,
					usDivider = 10UL
				};
				VOID_STATIC_ASSERT((uint32_t)OcrValue <= THwTimer1::maxCounterValue);
			};
		}
		#endif

	}
}
#elif defined(__AVR_ATmega32__) || defined (__AVR_ATmega64__) || defined (__AVR_ATmega649__)

#define CPH_HAS_TIMER0
#define CPH_INT_TIMER0_OCRA TIMER0_COMP_vect
#define CPH_HAS_TIMER1
#define CPH_INT_TIMER1_OCRA TIMER1_COMPA_vect
namespace cph {
	namespace Private {
		namespace timer0 {
			template <TimerPrescalar P>
			struct Timer0PrescalarData;

			#if defined(__AVR_ATmega32__) || defined (__AVR_ATmega64__)
			IO_REG_WRAPPER(TCCR0, Tccra, uint8_t);
			IO_REG_WRAPPER(OCR0, Ocra, uint8_t);
			IO_REG_WRAPPER(TIMSK, Timsk, uint8_t);
			IO_REG_WRAPPER(TIFR, Tifr, uint8_t);
			IO_REG_WRAPPER(TCNT0, Tcnt, uint8_t);
			enum {
				Ocie = OCIE0
			};
			#elif defined (__AVR_ATmega649__)
			IO_REG_WRAPPER(TCCR0A, Tccra, uint8_t);
			IO_REG_WRAPPER(OCR0A, Ocra, uint8_t);
			IO_REG_WRAPPER(TIMSK0, Timsk, uint8_t);
			IO_REG_WRAPPER(TIFR0, Tifr, uint8_t);
			IO_REG_WRAPPER(TCNT0, Tcnt, uint8_t);

			enum {
				Ocie = OCIE0A
			};
			#endif

			#if defined(__AVR_ATmega32__) || defined (__AVR_ATmega649__)
			DECLARE_TIMER_PRESCALAR(Timer0PrescalarData, Stopped, 0);
			DECLARE_TIMER_PRESCALAR(Timer0PrescalarData, Prescalar_1, (0 << CS02) | (0 << CS01) | (1 << CS00));
			DECLARE_TIMER_PRESCALAR(Timer0PrescalarData, Prescalar_8, (0 << CS02) | (1 << CS01) | (0 << CS00));
			DECLARE_TIMER_PRESCALAR(Timer0PrescalarData, Prescalar_64, (0 << CS02) | (1 << CS01) | (1 << CS00));
			DECLARE_TIMER_PRESCALAR(Timer0PrescalarData, Prescalar_256,
			                        (1 << CS02) | (0 << CS01) | (0 << CS00));
			DECLARE_TIMER_PRESCALAR(Timer0PrescalarData, Prescalar_1024,
			                        (1 << CS02) | (0 << CS01) | (1 << CS00));
			#endif
			#ifdef __AVR_ATmega64__
			DECLARE_TIMER_PRESCALAR(Timer0PrescalarData, Stopped, 0);
			DECLARE_TIMER_PRESCALAR(Timer0PrescalarData, Prescalar_1, (0 << CS02) | (0 << CS01) | (1 << CS00));
			DECLARE_TIMER_PRESCALAR(Timer0PrescalarData, Prescalar_8, (0 << CS02) | (1 << CS01) | (0 << CS00));
			DECLARE_TIMER_PRESCALAR(Timer0PrescalarData, Prescalar_64, (1 << CS02) | (0 << CS01) | (0 << CS00));
			DECLARE_TIMER_PRESCALAR(Timer0PrescalarData, Prescalar_256,
			                        (1 << CS02) | (1 << CS01) | (0 << CS00));
			DECLARE_TIMER_PRESCALAR(Timer0PrescalarData, Prescalar_1024,
			                        (1 << CS02) | (1 << CS01) | (1 << CS00));
			#endif

			template <TimerPrecision R, uint32_t FCPU>
			struct PrecisionInfo;


			struct THwTimer0 {

					static inline uint8_t _prescalarBits = Timer0PrescalarData<Stopped>::Bits;
					enum {
						maxCounterValue = 255
					};
					typedef void(*timerTickCallback_t)();
				private:
					static inline volatile timerTickCallback_t _cbOCRA_COMP = nullptr;

				public:
					static void IntOCRA_Handler() {
						if (_cbOCRA_COMP != nullptr) { _cbOCRA_COMP(); }
					}
					enum TimerMode {
						Normal = 0,
						FastPwm =	(1 << WGM00) | (1 << WGM01),
						Ctc =		(1 << WGM01) | (0 << WGM00),
						Mask =	   ~(1 << WGM01) | (1 << WGM00)
					};
					enum {
						PrescalarMask = ~((1 << CS02) | (1 << CS01) | (1 << CS00))
					};

					template <TimerPrecision P>
					static uint16_t tickPeriod_us() {
						return PrecisionInfo<P, 0>::usDivider;
					}


					static void reset() {
						Tcnt::Set(0);
					}

					static void setMode(TimerMode mode) {
						Tccra::AndOr(TimerMode::Mask, mode);
					}

					template <TimerPrescalar P>
					static void setPrescalar() {
						typedef Timer0PrescalarData<P> info;
						Tccra::AndOr(PrescalarMask, info::Bits);
					}

					static void setPrescalar(uint8_t bits) {
						Tccra::AndOr(PrescalarMask, bits);
					}

					template <TimerPrecision R, uint32_t FCPU>
					constexpr static void initPrecisionTimer(timerTickCallback_t cbTick, bool IsEnable) {
						Ocra::Set(PrecisionInfo<R, FCPU>::OcrValue);
						setMode(TimerMode::Ctc);
						_cbOCRA_COMP = cbTick;
						_prescalarBits = Timer0PrescalarData<PrecisionInfo<R, FCPU>::Prescalar>::Bits;
						if (IsEnable) { setPrescalar<PrecisionInfo<R, FCPU>::Prescalar>(); } else { setPrescalar<Stopped>(); }
					}
					/*
					template <TimerPrescalar P, class Period, uint32_t FCPU>
					constexpr static void initCustomTimer(timerTickCallback_t cbTick, const vd::chrono::any_t<Period>& time) {
						typedef TimerPrescalarValue<P> prescalar;
						//const uint32_t ocrValue = (usTickTime * FCPU) / 1000000 / (prescalar::value);
						const uint32_t ocrValue = ((FCPU) / (1000UL * Period::Denom) * Period::Num * time.count()) /
						                          (prescalar::value);
						VOID_STATIC_ASSERT(ocrValue == 0);
						VOID_STATIC_ASSERT(ocrValue <= maxCounterValue); //make Prescalar bigger!
						Ocra::Set(vd::lowByte(ocrValue));
						_cbOCRA_COMP = cbTick;
						setMode(TimerMode::Ctc);
						_prescalarBits = Timer0PrescalarData<P>::Bits;
						if (IsEnable) { setPrescalar<P>(); } else { setPrescalar<Stopped>(); }
					}
					*/
					template <uint32_t FCPU, class Period>
					constexpr static void initCustomTimer(const vd::chrono::any_t<Period>& tickTime,
					                                      timerTickCallback_t cbTick, bool IsEnable) {
						auto[prescalar,
						     ocrValue] = cph::Private::timer::calculatePrescalar <  maxCounterValue, FCPU, Period>(tickTime);
						assert(ocrValue <= maxCounterValue); //make Prescalar bigger
						Ocra::Set(vd::lowByte(ocrValue));
						_cbOCRA_COMP = cbTick;
						setMode(TimerMode::Ctc);
						_prescalarBits = getTimerPrescalarBits< Timer0PrescalarData>(prescalar);
						if (IsEnable) { setPrescalar(_prescalarBits); } else { setPrescalar<Stopped>(); }
					}

					static void enableTimer() {
						Tcnt::Set(0);
						Timsk::Or(1 << Ocie);
						setPrescalar(_prescalarBits);
					}

					static void disableTimer() {
						setPrescalar<Stopped>();
						Timsk::AndNot(1 << Ocie);
					}

			};



			template <uint32_t FCPU>
			struct PrecisionInfo<Resolution_1ms, FCPU> {
				static const TimerPrescalar Prescalar = TimerPrescalar::Prescalar_64;
				enum {
					OcrValue = FCPU / TimerPrescalarValue<Prescalar>::value / 1000UL,
					usDivider = 1000
				};
				VOID_STATIC_ASSERT(OcrValue <= 255);
			};
			template <uint32_t FCPU>
			struct PrecisionInfo<Resolution_100us, FCPU> {
				static const TimerPrescalar Prescalar = TimerPrescalar::Prescalar_8;

				enum {
					OcrValue = FCPU / TimerPrescalarValue<Prescalar>::value / 10000UL,
					usDivider = 100
				};
				VOID_STATIC_ASSERT(OcrValue <= 255);
			};
			template <uint32_t FCPU>
			struct PrecisionInfo<Resolution_10us, FCPU> {
				static const TimerPrescalar Prescalar = TimerPrescalar::Prescalar_1;

				enum {
					OcrValue = FCPU / TimerPrescalarValue<Prescalar>::value / 100000UL,
					usDivider = 10
				};
				VOID_STATIC_ASSERT(OcrValue <= 255);
			};
		}
//================================TIMER1=============================================================
		namespace timer1 {
			template <TimerPrescalar P>
			struct Timer1PrescalarData;
#if defined(__AVR_ATmega32__) || defined (__AVR_ATmega64__)
	IO_REG_WRAPPER(TCCR1A, Tccra, uint8_t);
	IO_REG_WRAPPER(TCCR1B, Tccrb, uint8_t);
	IO_REG_WRAPPER(OCR1AL, Ocral, uint8_t);
	IO_REG_WRAPPER(OCR1AH, Ocrah, uint8_t);
	IO_REG_WRAPPER(OCR1BL, Ocrbl, uint8_t);
	IO_REG_WRAPPER(OCR1BH, Ocrbh, uint8_t);
	IO_REG_WRAPPER(TIMSK, Timsk, uint8_t);
	IO_REG_WRAPPER(TIFR, Tifr, uint8_t);
	IO_REG_WRAPPER(TCNT1H, Tcnth, uint8_t);
	IO_REG_WRAPPER(TCNT1L, Tcntl, uint8_t);
#elif defined (__AVR_ATmega649__)
	IO_REG_WRAPPER(TCCR1A, Tccra, uint8_t);
	IO_REG_WRAPPER(TCCR1B, Tccrb, uint8_t);
	IO_REG_WRAPPER(OCR1AL, Ocral, uint8_t);
	IO_REG_WRAPPER(OCR1AH, Ocrah, uint8_t);
	IO_REG_WRAPPER(OCR1BL, Ocrbl, uint8_t);
	IO_REG_WRAPPER(OCR1BH, Ocrbh, uint8_t);
	IO_REG_WRAPPER(TIMSK0, Timsk, uint8_t);
	IO_REG_WRAPPER(TIFR0, Tifr, uint8_t);
	IO_REG_WRAPPER(TCNT1H, Tcnth, uint8_t);
	IO_REG_WRAPPER(TCNT1L, Tcntl, uint8_t);
#endif
			//both for mega32 and mega64
			DECLARE_TIMER_PRESCALAR(Timer1PrescalarData, Stopped, 0);
			DECLARE_TIMER_PRESCALAR(Timer1PrescalarData, Prescalar_1, (0 << CS12) | (0 << CS11) | (1 << CS10));
			DECLARE_TIMER_PRESCALAR(Timer1PrescalarData, Prescalar_8, (0 << CS12) | (1 << CS11) | (0 << CS10));
			DECLARE_TIMER_PRESCALAR(Timer1PrescalarData, Prescalar_64, (0 << CS12) | (1 << CS11) | (1 << CS10));
			DECLARE_TIMER_PRESCALAR(Timer1PrescalarData, Prescalar_256,
			                        (1 << CS12) | (0 << CS11) | (0 << CS10));
			DECLARE_TIMER_PRESCALAR(Timer1PrescalarData, Prescalar_1024,
			                        (1 << CS12) | (0 << CS11) | (1 << CS10));

			template <TimerPrecision R, uint32_t FCPU>
			struct Precision1Info;

			struct THwTimer1 {

					static inline uint8_t _prescalarBits = Timer1PrescalarData<Stopped>::Bits;
					enum {
						maxCounterValue = 65535
					};
					enum {
						PrescalarMask = ~((1 << CS12) | (1 << CS11) | (1 << CS10))
					};
					typedef void(*timerTickCallback_t)();
				private:
					static inline volatile timerTickCallback_t _cbOCRA_COMP = nullptr;

				public:
					static void IntOCRA_Handler() {
						if (_cbOCRA_COMP != nullptr) { _cbOCRA_COMP(); }
					}
					enum TimerMode {
						Normal = 0,
						FastPwm =	  ((0 << WGM13) << 8) | ((1 << WGM12) << 8) | (0 << WGM11) | (1 << WGM10),
						Ctc =		  ((0 << WGM13) << 8) | ((1 << WGM12) << 8) | (0 << WGM11) | (0 << WGM10),
						Mask =		~(((1 << WGM13) << 8) | ((1 << WGM12) << 8) | (1 << WGM11) | (1 << WGM10)),
					};


					template <TimerPrecision P>
					static uint16_t tickPeriod_us() {
						return Precision1Info<P, 0>::usDivider;
					}

					static void reset() {
						Tcnth::Set(0);
						Tcntl::Set(0);
					}

					static void setMode(TimerMode mode) {
						u16 currentMode = Tccra::Get() | (Tccrb::Get() << 8);
						currentMode &= TimerMode::Mask;
						currentMode |= mode;
						Tccra::Set(vd::lowByte(currentMode));
						Tccrb::Set(vd::hiByte(currentMode));
					}

					template <TimerPrescalar P>
					static void setPrescalar() {
						Tccrb::AndOr(PrescalarMask, Timer1PrescalarData<P>::Bits);
					}

					static void setPrescalar(uint8_t bits) {
						Tccrb::AndOr(PrescalarMask, bits);
					}

					template <TimerPrecision R, uint32_t FCPU>
					constexpr static void initPrecisionTimer(timerTickCallback_t cbTick, bool IsEnable) {
						Ocrah::Set(vd::hiByte(Precision1Info<R, FCPU>::OcrValue));
						Ocral::Set(vd::lowByte(Precision1Info<R, FCPU>::OcrValue));
						setMode(TimerMode::Ctc);
						_cbOCRA_COMP = cbTick;
						_prescalarBits = Timer1PrescalarData<Precision1Info<R, FCPU>::Prescalar>::Bits;
						if (IsEnable) { setPrescalar<Precision1Info<R, FCPU>::Prescalar>(); } else { setPrescalar<Stopped>(); }
					}
					/*
					template <TimerPrescalar P, class Period, uint32_t time, uint32_t FCPU>
					constexpr static void initCustomTimer(timerTickCallback_t cbTick,bool IsEnable) {
						typedef TimerPrescalarValue<P> prescalar;
						//const uint32_t ocrValue = (usTickTime * FCPU) / 1000000 / (prescalar::value);
						const uint32_t ocrValue = ((FCPU) / (1000UL * Period::Denom) * Period::Num * time) /
						                          (prescalar::value);
						VOID_STATIC_ASSERT(ocrValue <= maxCounterValue); //make Prescalar bigger!
						Ocrah::Set(vd::hiByte(ocrValue));
						Ocral::Set(vd::lowByte(ocrValue));
						setMode(TimerMode::Ctc);
						_cbOCRA_COMP = cbTick;
						_prescalarBits = Timer1PrescalarData<P>::Bits;
						if (IsEnable) { setPrescalar<P>(); } else { setPrescalar<Stopped>(); }
					}
					*/

					template <uint32_t FCPU, class Period>
					constexpr static void initCustomTimer(const vd::chrono::any_t<Period>& tickTime,
					                                      timerTickCallback_t cbTick, bool IsEnable) {
						auto[prescalar,
						     ocrValue] = cph::Private::timer::calculatePrescalar <  maxCounterValue, FCPU, Period>(tickTime);
						assert(ocrValue <= maxCounterValue); //make Prescalar bigger!
						Ocrah::Set(vd::hiByte(ocrValue));
						Ocral::Set(vd::lowByte(ocrValue));
						setMode(TimerMode::Ctc);
						_cbOCRA_COMP = cbTick;
						_prescalarBits = getTimerPrescalarBits< Timer1PrescalarData>(prescalar);
						if (IsEnable) { setPrescalar(_prescalarBits); } else { setPrescalar<Stopped>(); }
					}
					static void enableTimer() {
						Tcnth::Set(0);
						Tcntl::Set(0);
						Timsk::Or(1 << OCIE1A);
						setPrescalar(_prescalarBits);
					}

					static void disableTimer() {
						setPrescalar<Stopped>();
						Timsk::AndNot(1 << OCIE1A);
					}

			};



			template <uint32_t FCPU>
			struct Precision1Info<Resolution_1ms, FCPU> {
				static const TimerPrescalar Prescalar = TimerPrescalar::Prescalar_64;
				enum {
					OcrValue = FCPU / TimerPrescalarValue<Prescalar>::value / 1000UL,
					usDivider = 1000
				};
				VOID_STATIC_ASSERT((uint32_t)OcrValue <= THwTimer1::maxCounterValue);
			};
			template <uint32_t FCPU>
			struct Precision1Info<Resolution_100us, FCPU> {
				static const TimerPrescalar Prescalar = TimerPrescalar::Prescalar_8;

				enum {
					OcrValue = FCPU / TimerPrescalarValue<Prescalar>::value / 10000UL,
					usDivider = 100
				};
				VOID_STATIC_ASSERT((uint32_t)OcrValue <= THwTimer1::maxCounterValue);
			};
			template <uint32_t FCPU>
			struct Precision1Info<Resolution_10us, FCPU> {
				static const TimerPrescalar Prescalar = TimerPrescalar::Prescalar_1;

				enum {
					OcrValue = FCPU / TimerPrescalarValue<Prescalar>::value / 100000UL,
					usDivider = 10
				};
				VOID_STATIC_ASSERT((uint32_t)OcrValue <= THwTimer1::maxCounterValue);
			};
		}


	}
}
#else
//other MCU

#endif




