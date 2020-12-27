#pragma once
#include <void/static_assert.h>
#include <cph/ioreg.h>
#include "stm32_clock.h"


#define CPH_USE_PORTA
#define CPH_USE_PORTB
#define CPH_USE_PORTC
#define CPH_USE_PORTD
#define CPH_USE_PORTE
#define CPH_USE_PORTF

namespace cph {
	namespace io {
		class NativePortBase {
			public:
				enum {Width = 16};
				typedef uint16_t DataT;

				enum Configuration {
					Analog =  3,
					In =      0,
					Out =     1,
					AltFunc = 2
				};

				enum PullMode {
					NoPullUp = 0,
					PullUp   = 1,
					PullDown = 2
				};

				enum DriverType {
					PushPull  = 0,
					OpenDrain = 1
				};

				enum Speed {
					Slow    = 0,
					Fast    = 1,
					Fastest = 2
				};

				static inline unsigned UnpackConfig2bits(unsigned mask, unsigned value, unsigned configuration) {
					mask = (mask & 0xff00)     << 8 | (mask & 0x00ff);
					mask = (mask & 0x00f000f0) << 4 | (mask & 0x000f000f);
					mask = (mask & 0x0C0C0C0C) << 2 | (mask & 0x03030303);
					mask = (mask & 0x22222222) << 1 | (mask & 0x11111111);
					return (value & ~(mask * 0x03)) | mask * configuration;
				}

				static inline unsigned UnpackConfig4Bit(unsigned mask, unsigned value, unsigned configuration) {
					mask = (mask & 0xf0) << 12 | (mask & 0x0f);
					mask = (mask & 0x000C000C) << 6 | (mask & 0x00030003);
					mask = (mask & 0x02020202) << 3 | (mask & 0x01010101);
					return (value & ~(mask * 0x0f)) | mask * configuration;
				}
		};


		namespace Private {
			template<class GpioRegs, class ClkEnReg, int ID>
			class PortImplementation : public NativePortBase {
				public:
					static DataT Read() {
						return GpioRegs()->ODR;
					}
					static void Write(DataT value) {
						GpioRegs()->ODR = value;
					}
					static void ClearAndSet(DataT clearMask, DataT value) {
						GpioRegs()->BSRR = value | (uint32_t)clearMask << 16;
					}
					static void Set(DataT value) {
						GpioRegs()->BSRR = value;
					}
					static void Clear(DataT value) {
						GpioRegs()->BSRR = ((uint32_t)value << 16);
					}
					static void Toggle(DataT value) {
						GpioRegs()->ODR ^= value;
					}
					static DataT PinRead() {
						return GpioRegs()->IDR;
					}

					// constant interface

					template<DataT clearMask, DataT value>
					static void ClearAndSet() {
						GpioRegs()->BSRR = (value | (uint32_t)clearMask << 16);
					}

					template<DataT value>
					static void Toggle() {
						GpioRegs()->ODR ^= value;
					}

					template<DataT value>
					static void Set() {
						GpioRegs()->BSRR = value;
					}

					template<DataT value>
					static void Clear() {
						GpioRegs()->BSRR = ((uint32_t)value << 16);
					}

					// Configuration
					template<unsigned pin>
					static void SetPinConfiguration(Configuration configuration) {
						VOID_STATIC_ASSERT(pin < Width);
						GpioRegs()->MODER = UnpackConfig2bits(1 << pin, GpioRegs()->MODER, configuration);
					}
					static void SetConfiguration(DataT mask, Configuration configuration) {
						GpioRegs()->MODER = UnpackConfig2bits(mask, GpioRegs()->MODER, configuration);
					}

					template<DataT mask, Configuration configuration>
					static void SetConfiguration() {
						GpioRegs()->MODER = UnpackConfig2bits(mask, GpioRegs()->MODER, configuration);
					}

					static void SetSpeed(DataT mask, Speed speed) {
						GpioRegs()->OSPEEDR = UnpackConfig2bits(mask, GpioRegs()->OSPEEDR, speed);
					}

					static void SetPullUp(DataT mask, PullMode pull) {
						GpioRegs()->PUPDR = UnpackConfig2bits(mask, GpioRegs()->PUPDR, pull);
					}
					template<Configuration configuration>
					static void SetDriverType(DataT mask, DriverType driver) {
						GpioRegs()->OTYPER = (GpioRegs()->OTYPER & ~mask) | mask * configuration;
					}

					static void AltFuncNumber(DataT mask, uint8_t number) {
						GpioRegs()->AFR[0] = UnpackConfig4Bit(mask & 0xff,        GpioRegs()->AFR[0], number);
						GpioRegs()->AFR[1] = UnpackConfig4Bit((mask >> 8) & 0xff, GpioRegs()->AFR[1], number);
					}

					static void Enable() {
						ClkEnReg::Enable();
					}

					static void Disable() {
						ClkEnReg::Disable();
					}
					enum {Id = ID};
			};
		}

#define MAKE_PORT(GPIO_REGS, ClkEnReg, className, ID) \
	namespace Private{\
		IO_STRUCT_WRAPPER(GPIO_REGS, className ## Regs, GPIO_TypeDef);\
	}\
	typedef Private::PortImplementation<\
	Private::className ## Regs, \
	ClkEnReg,\
	ID> className;

#ifdef CPH_USE_PORTA
	MAKE_PORT(GPIOA, Clock::GpioaClock, Porta, 'A')
	#define CPH_HAS_PORTA
#endif

#ifdef CPH_USE_PORTB
	MAKE_PORT(GPIOB, Clock::GpiobClock, Portb, 'B')
	#define CPH_HAS_PORTB
#endif

#ifdef CPH_USE_PORTC
	MAKE_PORT(GPIOC, Clock::GpiocClock, Portc, 'C')
	#define CPH_HAS_PORTC
#endif

#ifdef CPH_USE_PORTD
	MAKE_PORT(GPIOD, Clock::GpiodClock, Portd, 'D')
	#define CPH_HAS_PORTD
#endif

#ifdef CPH_USE_PORTE
	MAKE_PORT(GPIOE, Clock::GpioeClock, Porte, 'E')
	#define CPH_HAS_PORTE
#endif

#ifdef CPH_USE_PORTF
	MAKE_PORT(GPIOF, Clock::GpiofClock, Portf, 'F')
	#define CPH_HAS_PORTF
#endif


//==================================================================================================
	}//namespace IO
}