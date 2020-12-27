#pragma once
#include <cph/protocols/onewire.h>
#include <cph/async.h>
namespace cph {
	//ds18b20_t<cph::UsartSync<cph::Usart0>>
	//or ds18b20_t<pinOneWire>
	template <typename USART_OR_PIN>
	struct ds18b20_t {
		private:
			using ONE_WIRE = TOneWire<USART_OR_PIN>;
			u08 _lowByte, _hiByte;
			enum class TCommands : uint8_t {
				SKIP_ROM = 0xCC,
				CONVERT = 0x44,
				READ = 0xBE
			};
			static constexpr vd::chrono::milli_t PAUSE_CONVERT = vd::chrono::milli_t(500);
			int16_t tconvert(uint8_t LSB, uint8_t MSB) {
				int16_t data;
				uint16_t temperature;
				temperature = LSB | (MSB << 8);

				if (temperature & 0x8000) {
					temperature = ~temperature + 1;
					data = 0 - (temperature / 16);
					return data;
				}

				data = temperature / 16;
				return data ;
			}
			uint8_t _state = 0;
		public:
			ds18b20_t(): _lowByte(0), _hiByte(0) {
				_state = 0;
			}
			void init() {
				_state = 0;
				ONE_WIRE::init();
			}
			void reset() {
				_state = 0;
			}
			TResultAsync<int16_t> getTemperature() {
				static cph::timer_t _tmrWait;
#define Try(x)\
	auto try_result = x; \
	if (!try_result.isComplete()) { break; } \
	if (!try_result.isSuccess()) {\
		\
		reset(); \
		return TResultAsync<int16_t>(try_result, 0); \
	} else

				switch (_state) {
				case 0: {
					Try(ONE_WIRE::sendReset()) _state++;
					break;
				}

				case 1: {
					Try(ONE_WIRE::writeByte((uint8_t)TCommands::SKIP_ROM)) _state++;
					break;
				}

				case 2: {
					Try(ONE_WIRE::writeByte((uint8_t)TCommands::CONVERT)) _state++;
					break;
				}

				case 3: {
					if (_tmrWait.isElapsed(PAUSE_CONVERT)) { _state++; }

					break;
				}

				case 4: {
					Try(ONE_WIRE::sendReset()) _state++;
					break;
				}

				case 5: {
					Try(ONE_WIRE::writeByte((uint8_t)TCommands::SKIP_ROM)) _state++;
					break;
				}

				case 6: {
					Try(ONE_WIRE::writeByte((uint8_t)TCommands::READ)) _state++;
					break;
				}

				case 7: {
					Try(ONE_WIRE::readByte()) {
						_state++;
						_lowByte = try_result.value();
					}
					break;
				}

				case 8: {
					Try(ONE_WIRE::readByte()) {
						_state = 0;
						_hiByte = try_result.value();
						auto t = tconvert(_lowByte, _hiByte);
						return TResultAsync<int16_t>(t);
					}
					break;
				}

				default:
					reset();
					break;
				}

				return TResultAsync<int16_t>();
			}
	};
}
