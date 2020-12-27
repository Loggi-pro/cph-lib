#pragma once
#include <unity/unity_fixture_classes.h>
#include <cph/timer.h>
class CphTest_Timer : UnityTestClass<CphTest_Timer> {
		typedef cph::SystemCounter TSystemTimer;
	public:
		static void setup() {}
		static void tearDown() {
			TSystemTimer::disable<cph::TIMER_FAKE>();
		}


		static void PrescalarAutoCalculation() {
			TSystemTimer::initCustom<cph::TIMER_FAKE, 16000000>(1_ms);
			TEST_ASSERT_EQUAL(cph::TimerPrescalar::Prescalar_64,
			                  TSystemTimer::Spy<cph::TIMER_FAKE>::getCurrentPrescalar());
			TEST_ASSERT_EQUAL(250, TSystemTimer::Spy<cph::TIMER_FAKE>::getOcrValue());
		}
		static void PrescalarAutoWithLongDuration_ShouldFail() {
			TSystemTimer::initCustom<cph::TIMER_FAKE, 16000000>(16_ms);
			TEST_ASSERT_EQUAL(cph::TimerPrescalar::Prescalar_1024,
			                  TSystemTimer::Spy<cph::TIMER_FAKE>::getCurrentPrescalar());
			//TSystemTimer::initCustom<vd::literal_milli<17>>(); SHOULD FAIL HERE
		}
		static void OcrOverflow_ShouldFail() {
			EXPECT_NO_ASSERT(
			    //PRESCALAR = 1024
			    SINGLE_ARG(TSystemTimer::initCustom<cph::TIMER_FAKE, 16000000>(16_ms))
			);
			EXPECT_ASSERT(
			    //PRESCALAR = 1024
			    SINGLE_ARG(TSystemTimer::initCustom<cph::TIMER_FAKE, 16000000>(17_ms))
			);
		}
		static void EqualityOfDifferentDurationTypes() {
			TSystemTimer::initCustom<cph::TIMER_FAKE, 16000000>(1000_us);
			TEST_ASSERT_EQUAL(cph::TimerPrescalar::Prescalar_64,
			                  TSystemTimer::Spy<cph::TIMER_FAKE>::getCurrentPrescalar());
			TEST_ASSERT_EQUAL(250, TSystemTimer::Spy<cph::TIMER_FAKE>::getOcrValue());
			TSystemTimer::initCustom<cph::TIMER_FAKE, 16000000>(1_ms);
			TEST_ASSERT_EQUAL(cph::TimerPrescalar::Prescalar_64,
			                  TSystemTimer::Spy<cph::TIMER_FAKE>::getCurrentPrescalar());
			TEST_ASSERT_EQUAL(250, TSystemTimer::Spy<cph::TIMER_FAKE>::getOcrValue());
		}


		static void run() {
			RUN_TEST_CASE_CLASS(CphTest_Timer, PrescalarAutoCalculation);
			RUN_TEST_CASE_CLASS(CphTest_Timer, PrescalarAutoWithLongDuration_ShouldFail);
			RUN_TEST_CASE_CLASS(CphTest_Timer, OcrOverflow_ShouldFail);
			RUN_TEST_CASE_CLASS(CphTest_Timer, EqualityOfDifferentDurationTypes);
		}
};