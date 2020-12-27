#pragma once

namespace cph {
	namespace Private {



		struct InternalReferenceStruct {
			#if defined(__ATmega16__) || defined(__AVR_ATmega16__)// atmega16
#define FREE_CONVERSION_RUN ADATE
			//typedef IO::PinList<Pa0, Pa1, Pa2, Pa3, Pa4, Pa5, Pa6, Pa7> AdcPins;
			static const uint16_t value = 0x028f; // 2.56V
			#elif defined(__ATmega8__) || defined(__AVR_ATmega8__) || defined(__ATmega8A__) // atmega8
			//typedef IO::PinList<Pc0, Pc1, Pc2, Pc3, Pc4, Pc5, NullPin, NullPin> AdcPins;
#define FREE_CONVERSION_RUN ADFR
			static const uint16_t value = 0x028f;// 2.56V
			#elif defined(__ATmega168__) || defined(__AVR_ATmega168__)  || defined(__AVR_ATmega168P__) || defined(__AVR_ATmega328P__)// atmega168
			//typedef IO::PinList<Pc0, Pc1, Pc2, Pc3, Pc4, Pc5> AdcPins;
#define FREE_CONVERSION_RUN ADATE
			static const uint16_t value = 0x011A; // 1.1V
			#else
#error ADC support is not implemented for this device yet.
			#endif

		};

		struct PlatformAdc {

			typedef uint16_t DataT;
			//	using namespace io;
			// ACD internal reference voltage in fixed point Q8 format
			static const uint16_t InternalReference = InternalReferenceStruct::value;

			static const unsigned TimeoutCycles = 128 * 25 / 3;




			enum Vref {
				External = 0 << REFS1 | 0 << REFS0,
				VCC = 0 << REFS1 | 1 << REFS0,
				Internal = 1 << REFS1 | 1 << REFS0
			};

			enum AdcDivider {
				Div2 = 0 << ADPS2 | 0 << ADPS1 | 1 << ADPS0,
				Div4 = 0 << ADPS2 | 1 << ADPS1 | 0 << ADPS0,
				Div8 = 0 << ADPS2 | 1 << ADPS1 | 1 << ADPS0,
				Div16 = 1 << ADPS2 | 0 << ADPS1 | 0 << ADPS0,
				Div32 = 1 << ADPS2 | 0 << ADPS1 | 1 << ADPS0,
				Div64 = 1 << ADPS2 | 1 << ADPS1 | 0 << ADPS0,
				Div128 = 1 << ADPS2 | 1 << ADPS1 | 1 << ADPS0
			};

			enum {
				DividerMask = 1 << ADPS2 | 1 << ADPS1 | 1 << ADPS0,
				MuxMask = 1 << MUX3 | 1 << MUX2 | 1 << MUX1 | 1 << MUX0,
				VrefMask = 1 << REFS1 | 1 << REFS0
			};

			enum ClockSource {
				MainClock = 0,
				AdcClock = 0
			};






			static DataT resolutionBits() {
				return 10;
			}

			static void setClockSource(ClockSource) {
				// Nothing to be done
			}

			static void setClockDivider(AdcDivider divider) {
				ADCSRA = (ADCSRA & DividerMask) | divider;
			}

			static void setVref(Vref ref) {
				ADMUX = (uint8_t)(ADMUX & ~VrefMask) | ref;
			}

			static void setChannel(uint8_t channel) {
				ADMUX = (uint8_t)(ADMUX & ~MuxMask) | (channel & MuxMask);
			}

			static void init(uint8_t channel, AdcDivider divider, Vref ref) {
				ADMUX = (uint8_t)(ADMUX & ~(MuxMask | VrefMask)) | (channel & MuxMask) | ref;
				ADCSRA = (uint8_t)(ADCSRA & DividerMask) | divider | 1 << ADEN;
			}
			template <typename pin>
			static void init(AdcDivider divider, Vref ref) {
				ADMUX = (uint8_t)((ADMUX & (uint8_t)~(MuxMask | VrefMask)) | (pin::Number & MuxMask) | ref);
				ADCSRA = (uint8_t)((ADCSRA & DividerMask) | divider | 1 << ADEN);
			}

			static void startContinuousConversions() {
				ADCSRA |= _BV(FREE_CONVERSION_RUN) | _BV(ADSC) | _BV(ADEN);
			}

			static void enableInterrupt() {
				ADCSRA |= _BV(ADIE) | _BV(ADEN);
			}

			static void startSingleConversion() {
				ADCSRA |= 1 << ADSC | 1 << ADEN;
			}

			static DataT read() {
				unsigned timeout = TimeoutCycles;

				while (!resultReady() && --timeout) {
					continue;
				}

				if (timeout == 0) {
					return 0xffff;
				}

				return ADC;
			}

			static bool resultReady() {
				return (ADCSRA & (1 << ADSC)) == 0;
			}

			static DataT singleConversion() {
				startSingleConversion();
				return read();
			}

			static void stop() {
				ADMUX = 0;
				ADCSRA = 0;
			}

			static void enable() {
				ADCSRA |= (uint8_t)(1 << ADEN);
			}

			static void disable() {
				ADCSRA &= (uint8_t)~(1 << ADEN);
			}

			static DataT getSeed() {
				startSingleConversion();

				while (!resultReady()) { continue; }

				DataT value = read();

				if (value == 0) { value++; }

				return value;
			}

		};

	}
}