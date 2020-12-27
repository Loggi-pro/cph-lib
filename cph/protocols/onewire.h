#pragma once
#include <cph/timer.h>
#include <cph/async.h>
#include <void/delay.h>
#include <void/atomic.h>
#include <void/chrono.h>
#include <void/type_traits.h>

namespace cph {
	namespace details_onewire {
		static constexpr auto FRAME_OVERTIME = 3000_us;
		template <typename T>
		class is_pin {
				template <typename C>
				static vd::true_type  test(decltype(&C::Number));
				template <typename C>
				static vd::false_type  test(...);

			public:
				using type = decltype(test<T>(0));
		};
		template <typename T>
		constexpr bool is_pin_v = is_pin<T>::type::value;

		template <class PIN_OR_USART, bool>
		struct TOneWireBase;

		//for Usart
		template <typename TUsart>
		struct TOneWireBase<TUsart, false> {

			private:
				enum {
					OW_0 = 0x00,
					OW_1 = 0xff,
					OW_R = 0xff
				};
				enum mode {modeReset, modeWork};
				enum state_t {
					StateSend,
					StateWait
				};
				static inline mode _currentMode = modeReset;
				static inline cph::timer_t _tmrOvertime;
				static inline bool _isOvertime = false;
				static void _switchToWorkMode() {
					_currentMode = modeWork;
					TUsart::template init<115200>();
					//vd::delay(1_ms);
					TUsart::reset();
				}
				static void _switchToResetMode() {
					_currentMode = modeReset;
					TUsart::template init<9600>();
					//vd::delay(1_ms);
					TUsart::reset();
				}
				static inline state_t _currentState = StateSend;
				static TResultAsync<uint8_t> rxtxByte(uint8_t byte) {
					static uint8_t _lastByte = 0;
					static uint8_t _counter = 0;

					if (_currentState == StateSend) {
						if (_currentMode != modeWork) {
							_switchToWorkMode();
						}

						for (uint8_t i = 0; i < 8; i++, byte >>= 1) {
							uint8_t iv = byte & 0x01 ? OW_1 : OW_0;
							TUsart::writeByte(iv);
						}

						_currentState = StateWait;
						_counter = 0;
						_lastByte = 0;
						_tmrOvertime.start();
					} else if (_currentState == StateWait) {
						uint8_t val;

						if (_tmrOvertime.isElapsed(details_onewire::FRAME_OVERTIME)) {
							_isOvertime = true;
							return TResultAsync<uint8_t>(TAsyncState::E_TIMEOUT);
						}

						if (!TUsart::readByte(&val)) {  return TResultAsync<uint8_t>(); }

						_tmrOvertime.start();
						_lastByte >>= 1;

						if (val == 0xFF) {
							_lastByte |= (0x01 << 7);
						}

						_counter++;

						if (_counter != 8) { return TResultAsync<uint8_t>(); }

						_currentState = StateSend;
						return TResultAsync<uint8_t>(_lastByte);
					}

					return TResultAsync<uint8_t>();
				}
			public:
				static bool isError() {
					bool error = _isOvertime;
					_isOvertime = false;
					return error;
				}
				static void init() {
					_switchToWorkMode();
					_isOvertime = false;
					_currentState = StateSend;
				}
				static TResultAsync<bool> sendReset() {
					if (_currentState == StateSend) {
						_switchToResetMode();
						TUsart::reset();
						TUsart::writeByte(0xF0);
						_currentState = StateWait;
						_tmrOvertime.start();
					} else if (_currentState == StateWait) {
						uint8_t presence;

						if (_tmrOvertime.isElapsed(details_onewire::FRAME_OVERTIME)) {
							_isOvertime = true;
							_currentState = StateSend;
							return TResultAsync<bool>(TAsyncState::E_TIMEOUT);
						}

						if (!TUsart::readByte(&presence)) { return TResultAsync<bool>(); }

						_switchToWorkMode();
						_currentState = StateSend;
						return TResultAsync<bool>(presence != 0xF0);
					}

					return TResultAsync<bool>();
				}


				static TVoidResultAsync writeByte(uint8_t byte) {
					return TVoidResultAsync(rxtxByte(byte).isSuccess());
				}

				static TResultAsync<uint8_t> readByte() {
					return rxtxByte(0xFF);
				}
				static TVoidResultAsync writeData(const vd::gsl::span<uint8_t>& data) {
					static uint8_t counter = 0;
					auto res = writeByte(data.data()[counter]);
					counter++;

					if (res.isFail()) {
						counter = 0;
						return res;
					}

					if (counter >= data.size()) {
						counter = 0;
						return TVoidResultAsync{true};
					}

					return TVoidResultAsync{};
				}




		};

		//for pin
		template <class PIN>
		struct TOneWireBase<PIN, true> {
			private:
				static inline cph::timer_t _tmrOvertime;
			public:
				static void _set0() {
					PIN::SetConfiguration(PIN::Port::Out);
					PIN::Clear();
				}
				static void _set1() {
					PIN::SetConfiguration(PIN::Port::In);
					PIN::Set();
				}

				static uint8_t _recByte() {
					uint8_t res = 0;

					for (uint8_t i = 0; i < 8; i++) {
						_set0();
						vd::delay(1_us);
						_set1();
						vd::delay(5_us);
						res >>= 1;

						if (PIN::IsSet()) { res |= 0x80; }

						vd::delay(64_us);
					}

					return res;
				}

				static void _writeByte(uint8_t byte) {
					for (uint8_t i = 0; i < 8; i++) {
						if (byte & 0x01) {
							_set0();//PORTB |=1<<0;
							vd::delay(7_us);
							_set1();
							vd::delay(63_us);
						} else {
							_set0();
							vd::delay(70_us);
							_set1();
						};

						byte >>= 1;
					};
				}


			public:
				static void init() {
					PIN::Port::Enable();
					_set1();
				}
				static TVoidResultAsync sendReset() {
					static uint8_t state = 0;

					switch (state) {
					case 0:
						ATOMIC_BLOCK() {
							_set0();
							vd::delay(480_us);
							_set1();
						}

						//cph::io::Pd3::SetConfiguration(cph::io::Pd3::Port::Out);
						//cph::io::Pd3::Set();
						state = 1;
						_tmrOvertime.start();
						break;

					case 1:
						if (_tmrOvertime.isElapsed(details_onewire::FRAME_OVERTIME)) { //no presense
							state = 0;
							//cph::io::Pd3::Clear();
							return TVoidResultAsync{ TAsyncState::E_TIMEOUT };
						}

						if (!PIN::IsSet()) { //exist
							state = 2;
							_tmrOvertime.start();
						}

						break;

					case 2:
						if (_tmrOvertime.isElapsed(240_us)) {
							state = 0;
							return TVoidResultAsync{ true };
						}

						break;
					}

					return TVoidResultAsync();
				}


				template <typename T>
				static TResultAsync<T> readData() {
					T res = 0;
					uint8_t* ptrRes = (uint8_t*)&res;
					ATOMIC_BLOCK() {
						for (uint8_t i = 0; i < sizeof(T); ++i) {
							ptrRes[i] = _recByte();
						}
					}
					return TResultAsync<T> { res };
				}

				static TVoidResultAsync writeData(const vd::gsl::span<uint8_t>& data) {
					ATOMIC_BLOCK() {
						for (uint8_t i = 0; i < data.size(); ++i) {
							_writeByte(data.data()[i]);
						}
					}
					return { true };
				}

				static TResultAsync<uint8_t> readByte() {
					uint8_t res = 0;
					ATOMIC_BLOCK() {
						res = _recByte();
					}
					return TResultAsync<uint8_t> { res };
				}
				static TVoidResultAsync writeByte(const uint8_t data) {
					_writeByte(data);
					return { true };
				}


		};
	}
	template <class PIN_OR_USART>
	using TOneWire = details_onewire::TOneWireBase<PIN_OR_USART,
	      details_onewire::is_pin_v<PIN_OR_USART>>;

}