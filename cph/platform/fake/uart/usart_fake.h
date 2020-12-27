#pragma once

#include <void\static_assert.h>
#include <void\assert.h>
#include <cph\containers\ring_buffer.h>

namespace cph {
	namespace Private {


		struct UsartFakeFlags {
			enum InterruptFlags {
				NoInterrupt = 0,
				RxNotEmptyInt = 1 << 1,
				TxCompleteInt = 1 << 2,
				TxEmptyInt = 1 << 3,
				LineIdleInt = 1 << 4,
				OverrunErrorInt = 1 << 5,
				ParityErrorInt = 1 << 6,
				LineBreakInt = 1 << 7,
				RxOrOverrunInt = 1 << 8,
				AllInterrupts =
				RxNotEmptyInt | TxCompleteInt | TxEmptyInt | LineIdleInt | OverrunErrorInt | ParityErrorInt |
				LineBreakInt | RxOrOverrunInt
			};

			enum Error {
				NoError = 0,
				OverrunError = 1 << 0,
				NoiseError = 1 << 2,
				FramingError = 1 << 3,
				ParityError = 1 << 4,
				ErrorMask = OverrunError | FramingError | ParityError
			};
			enum UsartDataBits {
				DataBits5 = 1 << 0,
				DataBits6 = 1 << 1,
				DataBits7 = 1 << 2,
				DataBits8 = 1 << 3,
				DataBits9 = 1 << 4,
			};

			enum UsartParity {
				NoneParity = 0,
				EvenParity = 1 << 5,
				OddParity = 1 << 6,
			};

			enum UsartMode {
				Disabled = 0,
				RxEnable = 1 << 7,
				TxEnable = 1 << 8,
				RxTxEnable = 1 << 9,
			};

			enum UsartStopBits {
				OneStopBit = 0,
				HalfStopBit = 1 << 10,
				TwoStopBits = 1 << 11,
				OneAndHalfStopBits = 1 << 12,
			};

		};


		template<class CLASS>
		struct UsartFake_Spy {

			static void setError(typename UsartFakeFlags::Error err) {
				CLASS::_error = err;
			}

			static bool isInited() {
				return CLASS::_isInited;
			}

			static void IntTxEmptyInterrupt() {
				u08 data;

				if (CLASS::_onDataTrancieveCallback(&data)) {
					CLASS::_outBuffer.push(data);
				}
			}

			static bool setReceiveByte(uint8_t data) {
				bool result = CLASS::_rxBuffer.push(data);
				assert(result); //TEST BUFFER OVERFLOW -> CHANGE SIZE
				CLASS::_onDataRecieveCallback(data);
				return result;
			}

			static void clearOutBuffer(void) {
				CLASS::_outBuffer.clear();
			}

			static bool getSendedByte(u08 *data) {
				if (CLASS::_outBuffer.empty()) { return false; }

				*data = CLASS::_outBuffer.pop();
				return true;
			}

			static uint32_t getBaudrate() {
				return CLASS::_baudrate;
			}

			static UsartFakeFlags::UsartDataBits getDataBits() {
				return CLASS::_databits;
			}

			static UsartFakeFlags::UsartParity getParity() {
				return CLASS::_parity;
			}

			static UsartFakeFlags::UsartMode getMode() {
				return CLASS::_mode;
			}

			static UsartFakeFlags::UsartStopBits getStopBits() {
				return CLASS::_stopBits;
			}


		};


		template<uint8_t BUFFER_SIZE>
		struct UsartFake_Base {
		private:
			template<uint8_t id>
			friend
			struct UsartFakeHelper; //fake functions
			static inline UsartFakeFlags::Error _error = UsartFakeFlags::NoError;
			static inline bool _isInited = false;
			static inline cph::RingBuffer<uint8_t, BUFFER_SIZE> _outBuffer =
					cph::RingBuffer<uint8_t, BUFFER_SIZE>{};
			static inline cph::RingBuffer<uint8_t, BUFFER_SIZE> _rxBuffer =
					cph::RingBuffer<uint8_t, BUFFER_SIZE>{};
			static inline uint32_t _baudrate = 0;
			static inline UsartFakeFlags::UsartDataBits _databits = UsartFakeFlags::DataBits8;
			static inline UsartFakeFlags::UsartParity _parity = UsartFakeFlags::NoneParity;
			static inline UsartFakeFlags::UsartMode _mode = UsartFakeFlags::RxTxEnable;
			static inline UsartFakeFlags::UsartStopBits _stopBits = UsartFakeFlags::OneStopBit;
		public:
			typedef void(*cbOnRecieve_t)(const uint8_t &value);

			typedef bool(*cbOnTrancieve_t)(uint8_t *data);

			typedef UsartFakeFlags Flags;
			using Spy = UsartFake_Spy<UsartFake_Base>;
			friend struct UsartFake_Spy<UsartFake_Base>;
		private:
			static inline cbOnRecieve_t _onDataRecieveCallback = nullptr;
			static inline cbOnTrancieve_t _onDataTrancieveCallback = nullptr;
		public:

			static bool writeReady() {
				return true;
			}

			static bool readReady() {
				return !_rxBuffer.is_empty();
			}

			static void write(uint8_t byte) {
				bool result = _outBuffer.push(byte);
				assert(result); ////TEST BUFFER OVERFLOW -> CHANGE SIZE
			}

			static uint8_t read() {
				uint8_t value;
				assert(!_rxBuffer.is_empty()); // NOTHING RECIEVED, CHANGE TEST!
				_rxBuffer.pop(&value);
				return value;
			}


			static void deinit() {
				_isInited = false;
			}

			//set mode of uart(data size, stop bits, parity,mode)
			template<uint32_t BAUD,
					typename Flags::UsartDataBits DB,
					typename Flags::UsartStopBits SB,
					typename Flags::UsartParity P,
					typename Flags::UsartMode M
			>
			static void init() {
				_isInited = true;
				_baudrate = BAUD;
				_databits = DB;
				_parity = P;
				_mode = M;
				_stopBits = SB;
			}

			static void init(uint32_t baudrate,
							 typename Flags::UsartDataBits db,
							 typename Flags::UsartStopBits sb,
							 typename Flags::UsartParity p,
							 typename Flags::UsartMode m) {
				_isInited = true;
				_baudrate = baudrate;
				_databits = db;
				_parity = p;
				_mode = m;
				_stopBits = sb;
			}

			static void enableInterrupt(typename UsartFakeFlags::InterruptFlags /*interrupts*/) {}

			static void disableInterrupt(typename UsartFakeFlags::InterruptFlags /*interrupts*/) {}

			static void StartTranceive() {}

			static bool IsRxInterrupt() {
				return true;
			}

			static bool IsTxEmptyInterrupt() {
				return true;
			}

			static bool IsTxCompleteInterrupt() {
				return true;
			}

			static typename UsartFakeFlags::Error getError() {
				typename UsartFakeFlags::Error tmp = _error;
				_error = Flags::NoError;
				return tmp;
			}

			static void setCallback(cbOnRecieve_t onDataRecieve,
									cbOnTrancieve_t onDataTrancieve) {
				_onDataRecieveCallback = onDataRecieve;
				_onDataTrancieveCallback = onDataTrancieve;
			}

		};


		using UsartFake = UsartFake_Base<255>;
	}
}

