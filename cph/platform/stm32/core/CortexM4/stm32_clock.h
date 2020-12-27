#pragma once

#include <cph/ioreg.h>
#include <stm32f303xc.h>
#include <void/assert.h>

namespace cph {
	namespace Clock {
		class ClockBase {
			protected:
				static const uint32_t ClockStartTimeout = 4000;

				static bool EnableClockSource(unsigned turnOnMask,  unsigned waitReadyMask) {
					uint32_t timeoutCounter = 0;
					RCC->CR |= turnOnMask;

					while (((RCC->CR & waitReadyMask) == 0) && (timeoutCounter < ClockStartTimeout)) {
						timeoutCounter++;
					}

					return (RCC->CR & waitReadyMask) != 0;
				}

				static bool DisablelockSource(unsigned turnOnMask,  unsigned waitReadyMask) {
					uint32_t timeoutCounter = 0;
					RCC->CR &= ~turnOnMask;

					while (((RCC->CR & waitReadyMask) != 0) && (timeoutCounter < ClockStartTimeout)) {
						timeoutCounter++;
					}

					return (RCC->CR & waitReadyMask) == 0;
				}
		};

		class HseClock : public ClockBase {
				static inline uint32_t _clock_frequency = 0;
			public:
				static uint32_t SrcClockFreq() {
					return _clock_frequency;
				}
				static void SetupClockFrequency(uint32_t clock_frequency) {
					_clock_frequency = clock_frequency;
				}
				static uint32_t GetDivider() { return 1; }

				static uint32_t GetMultipler() { return 1; }

				static uint32_t ClockFreq() {
					return SrcClockFreq();
				}

				static bool Enable() {
					bool result =  ClockBase::EnableClockSource(RCC_CR_HSEON, RCC_CR_HSERDY);

					/* config Flash for 2 latency cycles */
					/* Flash cant work on high frequency */
					if (result) {
						FLASH->ACR &= (uint32_t)(~FLASH_ACR_LATENCY);
						FLASH->ACR |= (uint32_t)FLASH_ACR_LATENCY_2;
					}

					return result;
				}

				static bool Disable() {
					return ClockBase::DisablelockSource(RCC_CR_HSEON, RCC_CR_HSERDY);
				}
		};

		class HsiClock : public ClockBase {
			public:
				static uint32_t SrcClockFreq() {
					return 8000000u;
				}

				static uint32_t GetDivider() { return 1; }

				static uint32_t GetMultipler() { return 1; }

				static uint32_t ClockFreq() {
					return SrcClockFreq();
				}

				static bool Enable() {
					return ClockBase::EnableClockSource(RCC_CR_HSION, RCC_CR_HSIRDY);
				}

				static bool Disable() {
					return ClockBase::DisablelockSource(RCC_CR_HSION, RCC_CR_HSIRDY);
				}
		};

		class LseClock : public ClockBase {
			public:
				static uint32_t SrcClockFreq() {
					return 32768;
				}

				static uint32_t GetDivider() { return 1; }

				static uint32_t GetMultipler() { return 1; }

				static uint32_t ClockFreq() {
					return SrcClockFreq();
				}

				static bool Enable() {
					assert(false);//NYI
					return false; // TODO
				}

				static bool Disable() {
					assert(false);//NYI
					return false; // TODO
				}
		};

		class LsiClock {
				inline static bool _wasInited = false;
			public:
				static uint32_t SrcClockFreq() {
					return 40000;
				}

				static uint32_t GetDivider() { return 1; }

				static uint32_t GetMultipler() { return 1; }

				static uint32_t ClockFreq() {
					return SrcClockFreq();
				}

				static bool Enable() {
					if (_wasInited) { return true; }

					_wasInited = true;
					const uint32_t ClockStartTimeout = 4000;
					uint32_t timeoutCounter = 0;
					RCC->CSR |= RCC_CSR_LSION;

					while (((RCC->CSR & RCC_CSR_LSIRDY) == 0) && (timeoutCounter < ClockStartTimeout)) {
						timeoutCounter++;
					}

					return (RCC->CSR & RCC_CSR_LSIRDY) != 0;
				}

				static bool Disable() {
					if (!_wasInited) { return true; }

					const uint32_t ClockStartTimeout = 4000;
					uint32_t timeoutCounter = 0;
					RCC->CSR &= ~RCC_CSR_LSION;

					while (((RCC->CSR & RCC_CSR_LSIRDY) != 0) && (timeoutCounter < ClockStartTimeout)) {
						timeoutCounter++;
					}

					_wasInited = false;
					return (RCC->CSR & RCC_CSR_LSIRDY) == 0;
				}
		};


		class PllClock : public ClockBase {
			public:
				enum ClockSource {
					Internal = RCC_CFGR_PLLSRC_HSI_DIV2,
					External = RCC_CFGR_PLLSRC_HSE_PREDIV,
				};

				static uint32_t SrcClockFreq() {
					if ((RCC->CFGR & RCC_CFGR_PLLSRC) == 0) {
						return HsiClock::ClockFreq();
					} else {
						return HseClock::ClockFreq();
					}
				}

				static uint32_t GetDivider() {
					if ((RCC->CFGR & RCC_CFGR_PLLSRC) == 0) {
						return 2;
					} else {
						return (RCC->CFGR2 & RCC_CFGR2_PREDIV_Msk) + 1;
					}
				}

				static uint32_t GetMultipler() {
					return ((RCC->CFGR & RCC_CFGR_PLLMUL_Msk) >> 18) + 2;
				}

				static void SetMultipler(uint8_t multiplier) {
					assert(multiplier >= 2);
					multiplier = static_cast<uint8_t>(multiplier - 2);

					if (multiplier > 15) {
						multiplier = 15;
					}

					RCC->CFGR = (RCC->CFGR & RCC_CFGR_PLLMUL_Msk) | (uint32_t)(multiplier << 18);
				}

				static void SetDivider(uint8_t divider) {
					divider--;

					if (divider > 15) {
						divider = 15;
					}

					RCC->CFGR2 = (RCC->CFGR2 & ~RCC_CFGR2_PREDIV_Msk) | (divider);
				}

				static void SelectClockSource(ClockSource clockSource) {
					RCC->CFGR = (RCC->CFGR & ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE) ) | (uint32_t)clockSource;
				}

				static uint32_t ClockFreq() {
					return SrcClockFreq() / GetDivider() * GetMultipler();
				}

				static bool Enable() {
					if ((RCC->CFGR & RCC_CFGR_PLLSRC) == 0) {
						if (!HsiClock::Enable()) {
							return false;
						}
					} else if (!HseClock::Enable()) {
						return false;
					}

					return ClockBase::EnableClockSource(RCC_CR_PLLON, RCC_CR_PLLRDY);
				}

				static void Disable() {
					ClockBase::DisablelockSource(RCC_CR_PLLON, RCC_CR_PLLRDY);
				}
		};


		class SysClock {
			public:
				enum ClockSource {
					Internal = 0,
					External = 1,
					Pll = 2
				};

			public:
				static bool SelectClockSource(ClockSource clockSource) {
					uint32_t clockStatusValue;
					uint32_t clockSelectMask;

					if (clockSource == Internal) {
						clockStatusValue = RCC_CFGR_SWS_HSI;
						clockSelectMask = RCC_CFGR_SW_HSI;

						if (!HsiClock::Enable()) {
							return false;
						}
					} else if (clockSource == External) {
						clockStatusValue = RCC_CFGR_SWS_HSE;
						clockSelectMask = RCC_CFGR_SW_HSE;

						if (!HseClock::Enable()) {
							return false;
						}
					} if (clockSource == Pll) {

						clockStatusValue = RCC_CFGR_SWS_PLL;
						clockSelectMask = RCC_CFGR_SW_PLL;

						if (!PllClock::Enable()) {
							return false;
						}
					} else {
						return false;
					}

					RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | clockSelectMask;

					while ((RCC->CFGR & RCC_CFGR_SWS) != clockStatusValue) {
					}

					return true;
				}

				static uint32_t ClockFreq() {
					uint32_t clockSrc = RCC->CFGR & RCC_CFGR_SWS;

					switch (clockSrc) {
					case 0:              return HsiClock::ClockFreq();

					case RCC_CFGR_SWS_0: return HseClock::ClockFreq();

					case RCC_CFGR_SWS_1: return PllClock::ClockFreq();
					}

					return 0;
				}

				static uint32_t SrcClockFreq() {
					return ClockFreq();
				}
		};


		template<class Reg, unsigned Mask, class ClockSrc>
		class ClockControl : public ClockSrc {
			public:
				static void Enable() {
					Reg::Or(Mask);
				}

				static void Disable() {
					Reg::And(~Mask);
				}
		};

		IO_BITFIELD_WRAPPER(RCC->CFGR, AhbPrescalerBitField, uint32_t, 4, 4);
		IO_BITFIELD_WRAPPER(RCC->CFGR, Apb1PrescalerBitField, uint32_t, 8, 3);
		IO_BITFIELD_WRAPPER(RCC->CFGR, Apb2PrescalerBitField, uint32_t, 11, 3);

		IO_BITFIELD_WRAPPER(RCC->CFGR, McoBitField, uint32_t, 24, 3);
		IO_BITFIELD_WRAPPER(RCC->CFGR3, Usart1ClockSwitch, uint32_t, RCC_CFGR3_USART1SW_Pos, 2);
		IO_BITFIELD_WRAPPER(RCC->CFGR3, Usart3ClockSwitch, uint32_t, RCC_CFGR3_USART3SW_Pos, 2);

		IO_BITFIELD_WRAPPER(RCC->CFGR2, Adc12Prescaller, uint32_t, 4, 5);
		IO_BITFIELD_WRAPPER(RCC->CFGR2, Adc34Prescaller, uint32_t, 9, 5);

		class AhbClock {
			public:
				enum Prescaller {
					Div1 = 0,
					Div2 = 0x08,
					Div4 = 0x09,
					Div8 = 0x0A,
					Div16 = 0x0B,
					Div64 = 0x0C,
					Div128 = 0x0D,
					Div256 = 0x0E,
					Div512 = 0x0F
				};

				static uint32_t SrcClockFreq() {
					return SysClock::ClockFreq();
				}

				static uint32_t ClockFreq() {
					uint32_t clock = SysClock::ClockFreq();
					uint8_t clockPrescShift[] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};
					uint8_t shiftBits = clockPrescShift[AhbPrescalerBitField::Get()];
					clock >>= shiftBits;
					return clock;
				}

				static void SetPrescaller(Prescaller prescaller) {
					AhbPrescalerBitField::Set((uint32_t)prescaller);
				}
		};

		class Apb1Clock {
			public:
				enum Prescaller {
					Div1 = 0,
					Div2 = 0x04,
					Div4 = 0x05,
					Div8 = 0x06,
					Div16 = 0x07,
				};

				static uint32_t SrcClockFreq() {
					return AhbClock::ClockFreq();
				}

				static uint32_t ClockFreq() {
					uint32_t clock = AhbClock::ClockFreq();
					uint8_t clockPrescShift[] = {0, 0, 0, 0, 1, 2, 3, 4};
					uint8_t shiftBits = clockPrescShift[Apb1PrescalerBitField::Get()];
					clock >>= shiftBits;
					return clock;
				}

				static void SetPrescaller(Prescaller prescaller) {
					Apb1PrescalerBitField::Set((uint32_t)prescaller);
				}
		};

		class Apb2Clock {
			public:
				enum Prescaller {
					Div1 = 0,
					Div2 = 0x04,
					Div4 = 0x05,
					Div8 = 0x06,
					Div16 = 0x07,
				};

				static uint32_t SrcClockFreq() {
					return AhbClock::ClockFreq();
				}

				static uint32_t ClockFreq() {
					uint32_t clock = AhbClock::ClockFreq();
					const uint8_t clockPrescShift[] = {0, 0, 0, 0, 1, 2, 3, 4};
					uint8_t shiftBits = clockPrescShift[Apb2PrescalerBitField::Get()];
					clock >>= shiftBits;
					return clock;
				}

				static void SetPrescaller(Prescaller prescaller) {
					Apb2PrescalerBitField::Set((uint32_t)prescaller);
				}
		};

		class Adc12ClockSource {
			public:

				enum ClockSource {
					Async = 0,
					AhbDiv1 = ADC12_CCR_CKMODE_0,
					AhbDiv2 = ADC12_CCR_CKMODE_1,
					AhbDiv4 = ADC12_CCR_CKMODE_0 | ADC12_CCR_CKMODE_1
				};

				enum Prescaller {
					Div1 = 16,
					Div2 = 17,
					Div4 = 18,
					Div6 = 19,
					Div8 = 20,
					Div10 = 21,
					Div12 = 22,
					Div16 = 23,
					Div32 = 24,
					Div64 = 25,
					Div128 = 26,
					Div256 = 27
				};

				static bool SelectClockSource(ClockSource clockSource) {
					ADC12_COMMON->CCR = (ADC12_COMMON->CCR & ~ADC12_CCR_CKMODE) | (uint32_t)clockSource;
					return true;
				}

				static void SetPrescaller(Prescaller prescaller) {
					Adc12Prescaller::Set((uint32_t)prescaller);
				}

				static uint32_t SrcClockFreq() {
					const uint16_t clockPresc[] = {1, 2, 4, 6, 8, 10, 12, 16, 32, 64, 128, 256};

					switch (ADC12_COMMON->CCR & ADC12_CCR_CKMODE) {
					case 0: {
						uint16_t divIndex = static_cast<uint16_t>(Adc12Prescaller::Get() - 16);

						if (divIndex > 11) {
							divIndex = 11;
						}

						return PllClock::ClockFreq() / clockPresc[divIndex];
					}

					case AhbDiv1: return AhbClock::ClockFreq();

					case AhbDiv2: return AhbClock::ClockFreq() >> 1;

					case AhbDiv4: return AhbClock::ClockFreq() >> 2;
					}

					return 0;
				}

				static uint32_t ClockFreq() {
					return SrcClockFreq();
				}
		};

		class Adc34ClockSource {
			public:

				enum ClockSource {
					AdcClock = 0,
					AhbDiv1 = ADC34_CCR_CKMODE_0,
					AhbDiv2 = ADC34_CCR_CKMODE_1,
					AhbDiv4 = ADC34_CCR_CKMODE_0 | ADC34_CCR_CKMODE_1
				};

				enum Prescaller {
					Div1 = 16,
					Div2 = 17,
					Div4 = 18,
					Div6 = 19,
					Div8 = 20,
					Div10 = 21,
					Div12 = 22,
					Div16 = 23,
					Div32 = 24,
					Div64 = 25,
					Div128 = 26,
					Div256 = 27
				};

				static bool SelectClockSource(ClockSource clockSource) {
					ADC34_COMMON->CCR = (ADC34_COMMON->CCR & ~ADC34_CCR_CKMODE) | (uint32_t)clockSource;
					return true;
				}

				static void SetPrescaller(Prescaller prescaller) {
					Adc34Prescaller::Set((uint32_t)prescaller);
				}

				static uint32_t SrcClockFreq() {
					const uint16_t clockPresc[] = {1, 2, 4, 6, 8, 10, 12, 16, 32, 64, 128, 256};

					switch (ADC34_COMMON->CCR & ADC12_CCR_CKMODE) {
					case 0: {
						uint16_t divIndex = static_cast<uint16_t>(Adc34Prescaller::Get() - 16);

						if (divIndex > 11)
						{ divIndex = 11; }

						return PllClock::ClockFreq() / clockPresc[divIndex];
					}

					case AhbDiv1: return AhbClock::ClockFreq();

					case AhbDiv2: return AhbClock::ClockFreq() >> 1;

					case AhbDiv4: return AhbClock::ClockFreq() >> 2;
					}

					return 0;
				}

				static uint32_t ClockFreq() {
					return SrcClockFreq();
				}
		};
		/*
		#define RCC_USART3CLKSOURCE_PCLK1        RCC_CFGR3_USART3SW_PCLK
		#define RCC_USART3CLKSOURCE_SYSCLK       RCC_CFGR3_USART3SW_SYSCLK
		#define RCC_USART3CLKSOURCE_LSE          RCC_CFGR3_USART3SW_LSE
		#define RCC_USART3CLKSOURCE_HSI          RCC_CFGR3_USART3SW_HSI
		*/
		class Usart1ClockSource {
			public:
				enum ClockSource {
					Pclk = RCC_CFGR3_USART1SW_PCLK,
					System = RCC_CFGR3_USART1SW_SYSCLK,
					Lse = RCC_CFGR3_USART1SW_LSE,
					Hsi = RCC_CFGR3_USART1SW_HSI,
				};

				static bool SelectClockSource(ClockSource source) {
					Usart1ClockSwitch::Set((uint32_t)source);
					return true;
				}

				static uint32_t SrcClockFreq() {
					switch (Usart1ClockSwitch::Get()) {
					case 0: return Apb1Clock::ClockFreq();

					case 1: return SysClock::ClockFreq();

					case 2: return LseClock::ClockFreq();

					case 3: return HsiClock::ClockFreq();
					}

					return 0;
				}

				static uint32_t ClockFreq() {
					return SrcClockFreq();
				}
		};


		class Usart3ClockSource {
			public:
				enum ClockSource {
					Pclk = RCC_CFGR3_USART3SW_PCLK >> RCC_CFGR3_USART3SW_Pos,
					System = RCC_CFGR3_USART3SW_SYSCLK >> RCC_CFGR3_USART3SW_Pos,
					Lse = RCC_CFGR3_USART3SW_LSE >> RCC_CFGR3_USART3SW_Pos,
					Hsi = RCC_CFGR3_USART3SW_HSI >> RCC_CFGR3_USART3SW_Pos,
				};

				static bool SelectClockSource(ClockSource source) {
					Usart3ClockSwitch::Set((uint32_t)source);
					return true;
				}

				static uint32_t SrcClockFreq() {
					switch (Usart3ClockSwitch::Get()) {
					case 0: return Apb1Clock::ClockFreq();

					case 1: return SysClock::ClockFreq();

					case 2: return LseClock::ClockFreq();

					case 3: return HsiClock::ClockFreq();
					}

					return 0;
				}

				static uint32_t ClockFreq() {
					return SrcClockFreq();
				}
		};


		IO_REG_WRAPPER(RCC->APB2ENR, PeriphClockEnable2, uint32_t);
		IO_REG_WRAPPER(RCC->APB1ENR, PeriphClockEnable1, uint32_t);
		IO_REG_WRAPPER(RCC->AHBENR, AhbClockEnableReg, uint32_t);

		typedef ClockControl<AhbClockEnableReg, RCC_AHBENR_DMA1EN, AhbClock> Dma1Clock;
		typedef ClockControl<AhbClockEnableReg, RCC_AHBENR_DMA2EN, AhbClock> Dma2Clock;
		typedef ClockControl<AhbClockEnableReg, RCC_AHBENR_SRAMEN, AhbClock> SramClock;
		typedef ClockControl<AhbClockEnableReg, RCC_AHBENR_FLITFEN, AhbClock> FlitfClock;
		typedef ClockControl<AhbClockEnableReg, RCC_AHBENR_CRCEN, AhbClock> CrcClock;
		typedef ClockControl<AhbClockEnableReg, RCC_AHBENR_GPIOAEN, AhbClock> GpioaClock;
		typedef ClockControl<AhbClockEnableReg, RCC_AHBENR_GPIOBEN, AhbClock> GpiobClock;
		typedef ClockControl<AhbClockEnableReg, RCC_AHBENR_GPIOCEN, AhbClock> GpiocClock;
		typedef ClockControl<AhbClockEnableReg, RCC_AHBENR_GPIODEN, AhbClock> GpiodClock;
		typedef ClockControl<AhbClockEnableReg, RCC_AHBENR_GPIOEEN, AhbClock> GpioeClock;
		typedef ClockControl<AhbClockEnableReg, RCC_AHBENR_GPIOFEN, AhbClock> GpiofClock;
		typedef ClockControl<AhbClockEnableReg, RCC_AHBENR_TSCEN, AhbClock> TsClock;
		typedef ClockControl<AhbClockEnableReg, RCC_AHBENR_ADC12EN, Adc12ClockSource> Adc12Clock;
		typedef ClockControl<AhbClockEnableReg, RCC_AHBENR_ADC34EN, Adc12ClockSource> Adc34Clock;

		typedef ClockControl<PeriphClockEnable2, RCC_APB2ENR_SYSCFGEN, Apb2Clock> SyscfgClock;
		typedef ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM1EN, Apb2Clock> Tim1Clock;
		typedef ClockControl<PeriphClockEnable2, RCC_APB2ENR_SPI1EN, Apb2Clock> Spi1Clock;
		typedef ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM8EN, Apb2Clock> Tim8Clock;
		typedef ClockControl<PeriphClockEnable2, RCC_APB2ENR_USART1EN, Usart1ClockSource> Usart1Clock;
		typedef ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM15EN, Apb2Clock> Tim15Clock;
		typedef ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM16EN, Apb2Clock> Tim16Clock;
		typedef ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM17EN, Apb2Clock> Tim17Clock;

		typedef ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM2EN, Apb1Clock> Tim2Clock;
		typedef ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM3EN, Apb1Clock> Tim3Clock;
		typedef ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM4EN, Apb1Clock> Tim4Clock;
		typedef ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM6EN, Apb1Clock> Tim6Clock;
		typedef ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM7EN, Apb1Clock> Tim7Clock;
		typedef ClockControl<PeriphClockEnable1, RCC_APB1ENR_WWDGEN, Apb1Clock> WwdgClock;
// typedef ClockControl<PeriphClockEnable1, RCC_APB1ENR_WWDGRST, Apb1Clock> IwdgClock;
		typedef ClockControl<PeriphClockEnable1, RCC_APB1ENR_SPI2EN, Apb1Clock> Spi2Clock;
		typedef ClockControl<PeriphClockEnable1, RCC_APB1ENR_SPI3EN, Apb1Clock> Spi3Clock;
		typedef ClockControl<PeriphClockEnable1, RCC_APB1ENR_USART2EN, Apb1Clock> Usart2Clock;
		typedef ClockControl<PeriphClockEnable1, RCC_APB1ENR_USART3EN, Apb1Clock> Usart3Clock;
//typedef ClockControl<PeriphClockEnable1, RCC_APB1ENR_UART3EN, Apb1Clock> Uart3Clock;
		typedef ClockControl<PeriphClockEnable1, RCC_APB1ENR_UART4EN, Apb1Clock> Uart4Clock;
		typedef ClockControl<PeriphClockEnable1, RCC_APB1ENR_I2C1EN, Apb1Clock> I2c1Clock;
		typedef ClockControl<PeriphClockEnable1, RCC_APB1ENR_I2C2EN, Apb1Clock> I2c2Clock;
		typedef ClockControl<PeriphClockEnable1, RCC_APB1ENR_USBEN, Apb1Clock> UsbClock;
		typedef ClockControl<PeriphClockEnable1, RCC_APB1ENR_CANEN, Apb1Clock> CanClock;
		typedef ClockControl<PeriphClockEnable1, RCC_APB1ENR_PWREN, Apb1Clock> PwrClock;
		typedef ClockControl<PeriphClockEnable1, RCC_APB1ENR_DAC1EN, Apb1Clock> DacClock;

		template <cph::Clock::PllClock::ClockSource source, uint32_t clock_freq>
		struct configure_as {};


		template <uint32_t clock_freq>
		struct configure_as<PllClock::ClockSource::External, clock_freq> { //external by default
			static bool  Mhz_72() {
				HseClock::SetupClockFrequency(clock_freq);

				if (HseClock::SrcClockFreq() != 8000000UL) { return false; } //NYI

				cph::Clock::PllClock::SetDivider(1);
				uint8_t multiplier = 72000000UL / clock_freq;
				cph::Clock::PllClock::SetMultipler(multiplier); //9 if clock_freq==8000000
				cph::Clock::PllClock::SelectClockSource(PllClock::ClockSource::External);

				if (!cph::Clock::PllClock::Enable()) { return false; }

				return cph::Clock::SysClock::SelectClockSource(cph::Clock::SysClock::ClockSource::Pll);
			}
		};
	}
}