#pragma once
#include <void/chrono.h>
#include "../../core/platform_clock.h" 
namespace cph {
	
	namespace Private {
	class wdt {
			static const uint16_t ReloadKey = 0xAAAA;
			static const uint16_t EnableKey = 0xCCCC;
			static const uint16_t AccessKey = 0x5555;

			static void _setValue(uint32_t prescaler, uint32_t value) {
				IWDG->KR = AccessKey;
				IWDG->PR = prescaler;
				IWDG->RLR = value;
				IWDG->KR = ReloadKey;
				IWDG->KR = EnableKey;
			};
		public:
			static const unsigned ClockFreq = 40000;
			
			static void start(uint32_t millisec) {
				const unsigned MaxReload = (1 << 12) - 1;
				unsigned reload  = (millisec * ClockFreq) / 1000;
				unsigned prescaller = 0;
				unsigned divider = 4;

				while (reload / divider > MaxReload) {
					divider *= 2;
					prescaller++;
				}

				reload /= divider;

				if (divider > 256) {
					prescaller = 7;
					reload = MaxReload;
				}
				cph::Clock::WwdgClock::Enable();
				_setValue(prescaller, reload);
			}
			template <typename Period>
			constexpr static void start(vd::chrono::any_t<Period> milli) {
				const unsigned MaxReload = (1 << 12) - 1;
				unsigned reload  = ( vd::chrono::milli_t(milli).count() * ClockFreq) / 1000;
				unsigned prescaller = 0;
				unsigned divider = 4;

				while (reload / divider > MaxReload) {
					divider *= 2;
					prescaller++;
				}

				reload /= divider;

				if (divider > 256) {
					prescaller = 7;
					reload = MaxReload;
				}
				cph::Clock::WwdgClock::Enable();
				_setValue(prescaller, reload);
			}

			static void disable() {
				
			}

			static void reset() {
				IWDG->KR = ReloadKey;
			}

			static bool causeReset() {
				return RCC->CSR & RCC_CSR_IWDGRSTF;
			}
	};
	}
}