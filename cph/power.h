#pragma once
#ifdef _AVR

	#include "platform/avr/core/platform_power.h"
#elif defined(_ARM)
	#include "platform/stm32/core/platform_power.h"
#endif
#include <void/chrono.h>
#pragma once

namespace cph {
	class power:public Private::power_extension {
		public:
			typedef cph::Private::SleepMode SleepMode;
			// Stop the CPU but all peripherals remains active.
			/*CpuOffMode
			// Stop the CPU but all peripherals running at lower freq if supported by traget MCU, if not its equ to CpuSleep.
			LowFreqMode


			// Stop the CPU and all peripherals exept that rinning asynchronously.
				//	Wakeup only by async events such as external interrupt,
				//	or interrupts from asynchronously running modules such as timers with external clock.
				//	External Reset or Watchdog Reset.
			AsyncPeriphMod

				// Stop the CPU and all peripherals clocks. Main clock remains running for faster wakeup.
				//	If not supported by target MCU this mode is equ to PowerDown
				//	Wakeup only by async events such as external interrupt.
				//	External Reset or Watchdog Reset.
			StopMode

			// Stop the CPU and all peripherals clocks. Main clock remains running for faster wakeup.
			//	If not supported by target MCU this mode is equ to StandbyMode
			//	Wakeup only by async events such as external interrupt.
			//	External Reset or Watchdog Reset.
			//После выхода из режима Stop тактирование надо снова конфигурировать.//
			StandbyMode
			*/
			static void sleep(SleepMode mode) {
				cph::Private::power::sleep(mode);
			}
			#ifdef _VOID_NATIVE_CPP17_CONSTEXPR_SUPPORT
			//For literal operator time support (like _ms,_h etc.)
			template <typename T>
			static void sleep(SleepMode mode,const T literal) {
				cph::Private::power::sleep<T>(mode);
			}

				template <SleepMode MODE,typename T>
			static void sleep(const T literal) {
				cph::Private::power::sleep<MODE,T>();
			}

			#endif


			template <SleepMode MODE>
			static void sleep() {
				cph::Private::power::sleep<MODE>();
			}



			// Stop the CPU and all peripherals clocks. Main clock remains running for faster wakeup.
			//	If not supported by target MCU this mode is equ to StandbyMode
			//	Wakeup only by async events such as external interrupt.
			//	External Reset or Watchdog Reset.
			//После выхода из режима Stop тактирование надо снова конфигурировать.//
			static void stop() {
				cph::Private::power::stop();
			}


			// Stop the CPU and all peripherals clocks. Main clock is source disabled.
			// All memory is reset. All gpio in Hi-Z mode.
			//	Wakeup only by async events such as external interrupt.
			//	External Reset or Watchdog Reset.
			static void standby() {
				cph::Private::power::standby();
			}

			// Need to be called from interrupts to exit sleep mode on some paltforms
			static void itExitSleepMode() {
				cph::Private::power::itExitSleepMode();
			}

	};
}
