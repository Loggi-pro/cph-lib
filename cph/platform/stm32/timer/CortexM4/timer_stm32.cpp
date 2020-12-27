//#include <void/platform_specific.h>
#include "m4_timer.h"
#ifdef _ARM
extern "C" {
	#ifdef CPH_HAS_TIMER_CORE
	void SysTick_Handler(void) {
		cph::Private::timer_core::THwTimerCore::IntHandler();
	}
	#endif
}
#ifdef CPH_HAS_TIMER1
extern "C" {

	void TIM1_UP_TIM16_IRQHandler(void) {
		cph::Private::timer1::THwTimer1::IntHandler();
	}
	/*
	void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim){
	  cph::Private::timer1::THwTimer1::otherIntHandler(htim);
	}

	*/
}
#endif
extern "C" {
	#ifdef CPH_HAS_TIMER_RTC
	void RTC_Alarm_IRQHandler(void) {
		cph::Private::timer_rtc::TTimer::Int_AlarmHandler();
	}

	void RTC_WKUP_IRQHandler(void) {
		/* USER CODE BEGIN RTC_WKUP_IRQn 0 */
		cph::Private::timer_rtc::TTimer::Int_WakeUpHandler();
		/* USER CODE END RTC_WKUP_IRQn 0 */
		/* USER CODE BEGIN RTC_WKUP_IRQn 1 */
		/* USER CODE END RTC_WKUP_IRQn 1 */
	}

	#endif


}


//TFAKE TIMER
#endif


