#pragma once
#include <cph/wake_timer.h>
namespace cph::Private {

	enum SleepMode {
		CpuOffMode,	// Stop the CPU but all peripherals remains active.
		LowFreqMode,	// Stop the CPU but all peripherals running at lower freq if supported by traget MCU, if not its equ to CpuOffMode.

		// Stop the CPU and all peripherals exept that rinning asynchronously.
		//	Wakeup only by async events such as external interrupt,
		//	or interrupts from asynchronously running modules such as timers with external clock.
		//	External Reset or Watchdog Reset
		AsyncPeriphMode,
		// Stop the CPU and all peripherals clocks. Main clock remains running for faster wakeup.
		//	If not supported by target MCU this mode is equ to StandbyMode
		//	Wakeup only by async events such as external interrupt.
		//	External Reset or Watchdog Reset.
		//После выхода из режима Stop тактирование надо снова конфигурировать.//
		StopMode,

		// Stop the CPU and all peripherals clocks. Main clock is source disabled.
		// All memory is reset. All gpio in Hi-Z mode.
		//	Wakeup only by async events such as external interrupt.
		//	External Reset or Watchdog Reset.
		StandbyMode
	};


	struct power {
		private:
			static void _cpuOffWaitForInterrupt() {
				HAL_SuspendTick();
				HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI); // WFI
				HAL_ResumeTick();
			}

			static void _cpuOffWaitForEvent() {
				HAL_SuspendTick();
				HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFE); // WFE
				HAL_ResumeTick();
			}

			static void _stopWaitForInterrupt() {
				HAL_SuspendTick();
				HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI); // WFI
				HAL_ResumeTick();
			}
			static void _stopWaitForEvent() {
				HAL_SuspendTick();
				HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFE); // WFE
				HAL_ResumeTick();
			}

			static void _stanbdy() {
				HAL_SuspendTick();
				HAL_PWR_EnterSTANDBYMode();
				HAL_ResumeTick();
			}

		public:
			template<SleepMode MODE>
			static void sleep() {
				switch (MODE) {
				case CpuOffMode:
					_cpuOffWaitForInterrupt(); break;

				case LowFreqMode:
					//TODO LowFreqMode
					_cpuOffWaitForInterrupt(); break;

				case AsyncPeriphMode:
					_cpuOffWaitForInterrupt(); break;

				case StopMode:
					_stopWaitForInterrupt(); break;

				case StandbyMode:
					_stanbdy(); break;
				}
			}

			static void sleep(SleepMode mode) {
				switch (mode) {
				case CpuOffMode:
					sleep<CpuOffMode>(); break;

				case LowFreqMode:
					sleep<LowFreqMode>(); break;

				case AsyncPeriphMode:
					sleep<AsyncPeriphMode>(); break;

				case StopMode:
					sleep<StopMode>(); break;

				case StandbyMode:
					sleep<StandbyMode>(); break;
				}
			}

			static void stop() {
				sleep<StopMode>();
			}

			static void standby() {
				sleep<StandbyMode>();
			}

			// Need to be called from interrupts to exit sleep mode on some paltforms
			static void itExitSleepMode() {
			}
	};

	struct power_extension {
		template <typename T>
		static void sleep(SleepMode mode, const T literal) {
			cph::stm32::TWakeUpTimer::initWakeUpEvery<T>(literal, nullptr);
			power::sleep(mode);
			cph::stm32::TWakeUpTimer::disable();
		}
		template <SleepMode MODE, typename T>
		static void sleep(const T literal) {
			cph::stm32::TWakeUpTimer::initWakeUpEvery<T>(literal, nullptr);
			power::sleep<MODE>();
			cph::stm32::TWakeUpTimer::disable();
		}
	};

}