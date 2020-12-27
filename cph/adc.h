#pragma once

#ifdef _AVR
	#include "platform/avr/adc/adc_impl.h"
#elif defined(_ARM)
	#include "platform/stm32/adc/adc_impl.h"
#endif
#include <void/assert.h>
namespace cph {




	class Adc {

		public:
			typedef typename cph::Private::PlatformAdc::DataT DataT;

			typedef typename cph::Private::PlatformAdc::Vref Vref;
			typedef typename cph::Private::PlatformAdc::AdcDivider AdcDivider;
			typedef typename cph::Private::PlatformAdc::ClockSource ClockSource;


			static DataT resolutionBits() {
				return cph::Private::PlatformAdc::resolutionBits();
			}

			static void setClockSource(ClockSource source) {
				cph::Private::PlatformAdc::setClockSource(source);
			}

			static void setClockDivider(AdcDivider divider) {
				cph::Private::PlatformAdc::setClockDivider(divider);
			}

			static void setVref(Vref ref) {
				cph::Private::PlatformAdc::setVref(ref);
			}

			static void setChannel(uint8_t channel) {
				cph::Private::PlatformAdc::setChannel(channel);
			}
			template<typename pin>
			static void init(AdcDivider divider = AdcDivider::Div32,
			                 Vref ref = Vref::VCC) {
				cph::Private::PlatformAdc::template init<pin>(divider, ref);
			}

			static void startContinuousConversions() {
				cph::Private::PlatformAdc::startContinuousConversions();
			}

			static void enableInterrupt() {
				cph::Private::PlatformAdc::enableInterrupt();
			}

			static void startSingleConversion() {
				cph::Private::PlatformAdc::startSingleConversion();
			}

			static DataT read() {
				return cph::Private::PlatformAdc::read();
			}
			template <typename T>
			static T readWithMap(T minValue, T maxValue) {
				assert(false);//NYT
				return (read() * (maxValue - minValue)) / (1 << cph::Private::PlatformAdc::resolutionBits()) +
				       minValue;
			}

			enum V_OPORN {
				Volate_3V3 = 0,
				Voltage_5V = 5
			};
			template<typename T, uint32_t R1, uint32_t R2, V_OPORN V>
			static T readWithVoltageDivider() {
				constexpr float Voporn = V == Volate_3V3 ? 3.3f : 5;
				constexpr float coeff = Voporn * (R1 + R2) / (R2 * (1 <<
				                        cph::Private::PlatformAdc::resolutionBits()));

				if (coeff < 1) {
					return read() / (1 / coeff);
				} else {
					return read() * coeff;
				}
			}

			static bool resultReady() {
				return cph::Private::PlatformAdc::resultReady();
			}

			static DataT singleConversion() {
				return cph::Private::PlatformAdc::singleConversion();
			}

			static void stop() {
				cph::Private::PlatformAdc::stop();
			}

			static void enable() {
				cph::Private::PlatformAdc::enable();
			}

			static void disable() {
				cph::Private::PlatformAdc::disable();
			}
			//init and read synchronously one value
			template <typename PIN>
			static DataT getSeed() {
				init<PIN>();
				return cph::Private::PlatformAdc::getSeed();
			}
	};

	typedef Adc Adc1;
}
