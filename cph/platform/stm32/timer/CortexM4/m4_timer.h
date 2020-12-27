#pragma once
#include <void/static_assert.h>
#include <void/assert.h>
#include <cph/ioreg.h>
#include <cph/timer/timer_types.h>
#include <void/chrono.h>
#include <void/delay.h>
#include "../../core/platform_clock.h"
namespace cph {
	namespace Private {
		struct TTimerHelperStub {};
	}
}

#define CPH_HAS_TIMER_CORE
#define CPH_HAS_TIMER1
// #define CPH_HAS_TIMER0
// #define CPH_INT_TIMER0_OCRA TIMER0_COMP_vect
// #define CPH_HAS_TIMER1
// #define CPH_INT_TIMER1_OCRA TIMER1_COMPA_vect

namespace cph {
	typedef void (*timerTickCallback_t)();
	namespace Private::timer_core {

		template <TimerPrecision R>
		struct PrecisionInfo;


		struct THwTimerCore {
			public:
				typedef uint32_t counter_t;

			private:
				static bool _initTimer(uint32_t tick_n, uint32_t TickPriority) {
					//need stmxxx_hal.c file
					if (HAL_SYSTICK_Config(tick_n) > 0U) {
						return false;
					}

					uwTickFreq = HAL_TICK_FREQ_1KHZ;

					/* Configure the SysTick IRQ priority */
					if (TickPriority < (1UL << __NVIC_PRIO_BITS)) {
						HAL_NVIC_SetPriority(SysTick_IRQn, TickPriority, 1U);
						uwTickPrio = TickPriority;
					} else {
						return false;
					}

					/* Return function status */
					return true;
				}
				inline static volatile timerTickCallback_t _cbTick = nullptr;
			public:
				static void IntHandler() {
					if (_cbTick != nullptr) { _cbTick(); }
				}

				static void reset() {
				}
				template <TimerPrecision R>
				static uint32_t tickPeriod_us() {
					return 1000000UL / PrecisionInfo<R>::divider; //tick in ms
				}

				template <TimerPrecision R, uint32_t FCPU>
				static void initPrecisionTimer(timerTickCallback_t cbTick, bool IsEnable) {
					uint32_t val = HAL_RCC_GetHCLKFreq() / PrecisionInfo<R>::divider;
					_cbTick = cbTick;
					_initTimer(val, TICK_INT_PRIORITY);

					if (IsEnable) {
						enableTimer();
					} else {
						disableTimer();
					}
				}

				template <class Period, uint32_t time, uint32_t FCPU>
				static void initCustomTimer(timerTickCallback_t cbTick, bool IsEnable) {
					_cbTick = cbTick;
					assert(false);//NYI
				}

				template <uint32_t FCPU, class Period>
				static void initCustomTimer(vd::chrono::any_t<Period> tickTime, timerTickCallback_t cbTick) {
					_cbTick = cbTick;
					assert(false);//NYI
				}

				static void enableTimer() {
				}

				static void disableTimer() {
				}

		};


		template <>
		struct PrecisionInfo<Resolution_1ms> {
			static const uint32_t divider = 1000UL;
		};
		template <>
		struct PrecisionInfo<Resolution_100us> {
			static const uint32_t divider = 10000UL;
		};
		template <>
		struct PrecisionInfo<Resolution_10us> {
			static const uint32_t divider = 100000UL;
		};
	}

	namespace Private::timer_rtc {
#ifndef HAL_RTC_MODULE_ENABLED
	#warning ENABNLE RTC MODULE IN HAL
#endif
		#ifdef HAL_RTC_MODULE_ENABLED
#define CPH_HAS_TIMER_RTC


		struct TTimer {
			public:
				typedef uint32_t counter_t;
				typedef void (*timerTickCallback_t)();
			private:
				inline static volatile timerTickCallback_t _cbAlarm = nullptr;
				inline static volatile timerTickCallback_t _cbWakeUp = nullptr;
				inline static RTC_HandleTypeDef hrtc;
				inline static bool _inited = false;
			public:
				static void Int_AlarmHandler() {
					HAL_RTC_AlarmIRQHandler(&hrtc);

					if (_cbAlarm != nullptr) { _cbAlarm(); }
				}
				static void Int_WakeUpHandler() {
					HAL_RTCEx_WakeUpTimerIRQHandler(&hrtc);

					if (_cbWakeUp != nullptr) { _cbWakeUp(); }
				}

				static void _setupLsiClock() {
					cph::Clock::LsiClock::Enable();
					RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
					HAL_RCCEx_GetPeriphCLKConfig(&PeriphClkInit);
					PeriphClkInit.PeriphClockSelection	|= RCC_PERIPHCLK_RTC;
					PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
					HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
				}

			private:
				static bool _initTimer() {
					RTC_TimeTypeDef sTime = {0};
					RTC_DateTypeDef sDate = {0};
					hrtc.Instance = RTC;
					hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
					hrtc.Init.AsynchPrediv = 127;
					hrtc.Init.SynchPrediv = 311;
					hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
					hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
					hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
					__HAL_RCC_RTC_ENABLE();
					HAL_NVIC_SetPriority(RTC_WKUP_IRQn, 0, 0);
					HAL_NVIC_EnableIRQ(RTC_WKUP_IRQn);
					HAL_RTC_Init(&hrtc);
					sTime.Hours = 1;
					sTime.Minutes = 0;
					sTime.Seconds = 0;
					sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
					sTime.StoreOperation = RTC_STOREOPERATION_RESET;
					HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
					sDate.WeekDay = RTC_WEEKDAY_MONDAY;
					sDate.Month = RTC_MONTH_JANUARY;
					sDate.Date = 1;
					sDate.Year = 0;
					HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
					/** Enable the Alarm A
					*/
					/*
					RTC_AlarmTypeDef sAlarm = {0};
					sAlarm.AlarmTime.Hours = 1;
					sAlarm.AlarmTime.Minutes = 0;
					sAlarm.AlarmTime.Seconds = 10;
					sAlarm.AlarmTime.SubSeconds = 0;
					sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
					sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
					sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY|RTC_ALARMMASK_HOURS;
					sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
					sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
					sAlarm.AlarmDateWeekDay = 1;
					sAlarm.Alarm = RTC_ALARM_A;
					HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN);
					HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 0, 0);
					HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
					*/
					return true;
				}
				template<uint32_t RTCFREQ, uint32_t sPeriod>
				static void _initWakeUp() {
					//constexpr uint32_t counter = ((sPeriod)*(RTCFREQ/16UL));
					constexpr uint32_t counter = sPeriod;
					VOID_STATIC_ASSERT((counter > 0) && (counter <= 65535));
					HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, counter, RTC_WAKEUPCLOCK_CK_SPRE_16BITS); //in seconds
					__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
					/* RTC interrupt Init */
				}

			public:
				static void disable() {
					HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
				}
				template <class literalTime>
				static void initWakeUpEvery(timerTickCallback_t cbWakeUp) {
					if (!_inited) {
						_setupLsiClock();
						_initTimer();
						_inited = true;
					}

					_cbWakeUp = cbWakeUp;
					_initWakeUp<40000, literalTime::to_seconds::static_value>();
				}
				template <class T>
				constexpr static void initWakeUpEvery(T literalTime, timerTickCallback_t cbWakeUp) {
					initWakeUpEvery<T>(cbWakeUp);
				}
			public:

		};
		#endif

	}

	namespace Private::timer1 {

		template <TimerPrecision R>
		struct PrecisionInfo;

		template <>
		struct PrecisionInfo<Resolution_1ms> {
			static const uint32_t Prescaler = 999UL;

			static constexpr uint32_t Period(uint32_t clock) { return clock / 1000000UL - 1UL;}
		};
		template <>
		struct PrecisionInfo<Resolution_100us> {
			static const uint32_t Prescaler = 99UL;
			static constexpr uint32_t Period(uint32_t clock) { return clock / 1000000UL - 1UL;}
		};
		template <>
		struct PrecisionInfo<Resolution_10us> {
			static const uint32_t Prescaler = 9UL;
			static constexpr uint32_t Period(uint32_t clock) { return clock / 1000000UL - 1UL;}
		};

		struct THwTimer1 {
			public:
				typedef uint32_t counter_t;

			private:
				static inline TIM_HandleTypeDef _handle;
				static bool _initTimer(uint32_t TickPriority) {
					cph::Clock::Tim1Clock::Enable();
					TIM_ClockConfigTypeDef sClockSourceConfig = {0};
					TIM_MasterConfigTypeDef sMasterConfig = {0};

					if (HAL_TIM_Base_Init(&_handle) != HAL_OK) {
						return false;
					}

					sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;

					if (HAL_TIM_ConfigClockSource(&_handle, &sClockSourceConfig) != HAL_OK) {
						return false;
					}

					sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
					sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
					sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;

					if (HAL_TIMEx_MasterConfigSynchronization(&_handle, &sMasterConfig) != HAL_OK) {
						return false;
					}

					/* Configure the SysTick IRQ priority */
					HAL_NVIC_SetPriority(TIM1_UP_TIM16_IRQn, TickPriority, 0);
					return true;
				}
				template<typename Period>
				constexpr static uint32_t prescaler(uint32_t clock, const vd::chrono::any_t<Period>& time,
				                                    uint32_t period,
				                                    uint32_t RCR) {
					typedef typename vd::chrono::any_t<Period>::PeriodInfo PeriodTypedef;
					return ((clock / ((period + 1) * (RCR + 1))) * time.count() * PeriodTypedef::Num /
					        PeriodTypedef::Denom) / 1000UL - 1 + 1;
				}

				template<typename Period>
				constexpr static uint32_t period(uint32_t clock, const vd::chrono::any_t<Period>& time,
				                                 uint32_t prescaler,
				                                 uint32_t RCR) {
					typedef typename vd::chrono::any_t<Period>::PeriodInfo PeriodTypedef;
					uint32_t tikz = clock / (prescaler + 1);
					uint32_t tikz2 = tikz / PeriodTypedef::Denom;
					return (tikz2 * time.count() * PeriodTypedef::Num) / (RCR + 1) / 1000UL - 1;
				}


				template <typename Period>
				constexpr static void _setPrescalar(const vd::chrono::any_t<Period>& time) {
					const uint32_t RCR = 0;
					uint32_t clock = cph::Clock::Tim1Clock::ClockFreq();
					_handle.Init.CounterMode = TIM_COUNTERMODE_UP;
					_handle.Init.Prescaler = prescaler(clock, time, 65534, RCR);
					_handle.Init.Period = period(clock, time, _handle.Init.Prescaler, RCR);
					_handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
					_handle.Init.RepetitionCounter = RCR;
					_handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
				}

				template <TimerPrecision R>
				constexpr static void _setPrescalar() {
					const uint32_t RCR = 0;
					uint32_t clock = cph::Clock::Tim1Clock::ClockFreq();
					_handle.Init.CounterMode = TIM_COUNTERMODE_UP;
					_handle.Init.Prescaler = PrecisionInfo<R>::Prescaler;
					_handle.Init.Period = PrecisionInfo<R>::Period(clock);
					_handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
					_handle.Init.RepetitionCounter = RCR;
					_handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
				}

				inline static volatile timerTickCallback_t _cbTick = nullptr;
			public:
				static void IntHandler() {
					if (__HAL_TIM_GET_FLAG(&_handle, TIM_FLAG_UPDATE) != RESET) {
						if (__HAL_TIM_GET_IT_SOURCE(&_handle, TIM_IT_UPDATE) != RESET) {
							__HAL_TIM_CLEAR_FLAG(&_handle, TIM_FLAG_UPDATE);

							if (_cbTick != nullptr) {
								_cbTick();
							}
						}
					}

					HAL_TIM_IRQHandler(&_handle);
				}
				/*static void otherIntHandler(TIM_HandleTypeDef* htim) {
					if (htim->Instance == TIM1) {
						if (_cbTick != nullptr) {
							_cbTick();
						}
					}
				}
				*/
				static void reset() {
					_handle.Instance->CNT = 0;
					//HAL_TIM_Base_Stop_IT(&_handle);
					//HAL_TIM_Base_Start_IT(&_handle);
				}
				template <TimerPrecision R>
				static uint32_t tickPeriod_us() {
					assert(false); //NYI
					return 0;
				}

				template <TimerPrecision R, uint32_t FCPU>
				constexpr static void initPrecisionTimer(timerTickCallback_t cbTick, bool IsEnable) {
					_cbTick = cbTick;
					_handle.Instance = TIM1;
					_setPrescalar<R>();
					_initTimer(TICK_INT_PRIORITY);

					if (IsEnable) {
						enableTimer();
					}
				}


				template <uint32_t FCPU, class Period>
				constexpr static void initCustomTimer(const vd::chrono::any_t<Period>& tickTime,
				                                      timerTickCallback_t cbTick, bool IsEnable) {
					_cbTick = cbTick;
					_handle.Instance = TIM1;
					_setPrescalar(tickTime);
					_initTimer(TICK_INT_PRIORITY);

					if (IsEnable) {
						enableTimer();
					}
				}

				static void enableTimer() {
					HAL_NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);
					HAL_TIM_Base_Start_IT(&_handle);
				}

				static void disableTimer() {
					HAL_NVIC_DisableIRQ(TIM1_UP_TIM16_IRQn);
					HAL_TIM_Base_Stop_IT(&_handle);
				}
				static void deinit() {
					cph::Clock::Tim1Clock::Disable();
				}

		};



	}

}
