#pragma once

#include <void/gsl.h>
#include <void/static_assert.h>

#ifdef _STM8
#include <cph/platform/stm8/uart/usart_stm8.h>
#elif defined _AVR
//750byte
//���� ����� ������, �� ����� ������� ���� ������(-300 ����) ���� ��������(-150 ����)
#include <cph/platform/avr/uart/uart.h>

#elif defined(WIN32)
#elif defined(_ARM)
#include <cph/platform/stm32/uart/platform_uart.h>
#endif

#include <cph/platform/fake/uart/usart_fake.h>

namespace cph {

	using UsartFake = cph::Private::UsartFake;
	//Usart0
	//Usart1
	//Usart2
	//Usart3

#ifndef _ARM

	template<class HardwareUart>
	class UsartSync {
		UsartSync() = delete;

	public:
		typedef HardwareUart Hardware;
		typedef typename Hardware::Flags Flags;
		typedef typename HardwareUart::Spy Spy;


		template<uint32_t BAUDRATE,
				typename Flags::UsartDataBits DB = Flags::DataBits8,
				typename Flags::UsartStopBits SB = Flags::OneStopBit,
				typename Flags::UsartParity P = Flags::NoneParity,
				typename Flags::UsartMode M = Flags::RxTxEnable
		>
		static void init() {
			Hardware::template init<BAUDRATE, DB, SB, P, M>();
		}

		static void init(uint32_t baudrate,
						 typename Flags::UsartDataBits db = Flags::DataBits8,
						 typename Flags::UsartStopBits sb = Flags::OneStopBit,
						 typename Flags::UsartParity p = Flags::NoneParity,
						 typename Flags::UsartMode m = Flags::RxTxEnable) {
			Hardware::init(baudrate, db, sb, p, m);
		}

		static void setMode(typename Flags::UsartMode m) {
			Hardware::setMode(m);
		}

		static typename Flags::UsartMode getMode() {
			return Hardware::getMode();
		}

		static void reset() {
		}

		static void deinit() {
			Hardware::deinit();
		}


		static bool readByte(uint8_t *out) {
			if (!Hardware::readReady()) { return false; }

			*out = Hardware::read();
			return true;
		}

		static bool writeByte(uint8_t c) {
			if (!Hardware::writeReady()) { return false; }

			Hardware::write(c);
			return true;
		}


		static typename Flags::Error getError() {
			return Hardware::getError();
		}

	};

#endif

	//Asynchronous Usart

	template<class HardwareUart, uint8_t OUT_BUFFER_SIZE = 255>
	class UsartAsync {
		UsartAsync() {};
	public:
		typedef HardwareUart Hardware;
		typedef typename Hardware::Flags Flags;
		typedef typename HardwareUart::Spy Spy;
	private:
		static inline auto _buffer = cph::RingBuffer<uint8_t, OUT_BUFFER_SIZE>{};

		static bool _cbTranceive(uint8_t *byte) {
			if (_buffer.empty()) { return false; }

			*byte = _buffer.pop();
			return true;
		}

	public:


		template<uint32_t BAUDRATE,
				typename Flags::UsartDataBits DB = Flags::DataBits8,
				typename Flags::UsartStopBits SB = Flags::OneStopBit,
				typename Flags::UsartParity P = Flags::NoneParity,
				typename Flags::UsartMode M = Flags::RxTxEnable
		>
		static void init() {
			Hardware::template init<BAUDRATE, DB, SB, P, M>();
			_buffer.clear();
			Hardware::setCallback(nullptr, &_cbTranceive);
#ifndef _ARM
			Hardware::enableInterrupt(Hardware::Flags::RxNotEmptyInt);
#endif
		}

		static void init(uint32_t baudrate,
						 typename Flags::UsartDataBits db = Flags::DataBits8,
						 typename Flags::UsartStopBits sb = Flags::OneStopBit,
						 typename Flags::UsartParity p = Flags::NoneParity,
						 typename Flags::UsartMode m = Flags::RxTxEnable) {
			Hardware::init(baudrate, db, sb, p, m);
			_buffer.clear();
			Hardware::setCallback(nullptr, &_cbTranceive);
#ifndef _ARM
			Hardware::enableInterrupt(Hardware::Flags::RxNotEmptyInt);
#endif
		}

		static void setMode(typename Flags::UsartMode m) {
			Hardware::setMode(m);
		}

		static typename Flags::UsartMode getMode() {
			return Hardware::getMode();
		}

		static void deinit() {
			Hardware::disableInterrupt(Flags::AllInterrupts);
			Hardware::deinit();
		}

		static bool writeData(const vd::gsl::span<uint8_t> &data) {
			if (data.size() > (uint16_t) (_buffer.capacity() - _buffer.count())) { return false; }

			for (size_t i = 0; i < data.size(); i++) {
				uint8_t byte = data.data()[i];
				_buffer.push(byte);
			}

			Hardware::StartTranceive();
			return true;
		}

		static bool writeByte(uint8_t byte) {
			bool res = _buffer.push(byte);

			if (res) {
				Hardware::StartTranceive();
			}

			return res;
		}

		static bool isSended() {
			return _buffer.is_empty();
		}

		static void reset() {
			_buffer.clear();
		}

		static void setOnReceive(typename Hardware::cbOnRecieve_t onDataReceiveCallback) {
			Hardware::setCallback(onDataReceiveCallback, &_cbTranceive);
		}

		static typename Flags::Error getError() {
			return Hardware::getError();
		}
	};




	//Make async usart synchronous

	template<typename USART_ASYNC, uint8_t IN_BUFFER_SIZE>
	class AdapterUsartSynchronized : public USART_ASYNC {
	private:
		static inline auto _inBuffer = cph::RingBuffer<uint8_t, IN_BUFFER_SIZE>{};

		static void _cbOnReceive(const uint8_t &val) {
			_inBuffer.push(val);
		}

	public:
		template<uint32_t BAUDRATE,
				typename USART_ASYNC::Flags::UsartDataBits DB = USART_ASYNC::Flags::DataBits8,
				typename USART_ASYNC::Flags::UsartStopBits SB = USART_ASYNC::Flags::OneStopBit,
				typename USART_ASYNC::Flags::UsartParity P = USART_ASYNC::Flags::NoneParity,
				typename USART_ASYNC::Flags::UsartMode M = USART_ASYNC::Flags::RxTxEnable
		>
		static void init() {
			USART_ASYNC::template init<BAUDRATE, DB, SB, P, M>();
			USART_ASYNC::setOnReceive(&_cbOnReceive);
		}

		static void reset() {
			_inBuffer.clear();
			USART_ASYNC::reset();
		}

		static void init(uint32_t baudrate,
						 typename USART_ASYNC::Flags::UsartDataBits db = USART_ASYNC::Flags::DataBits8,
						 typename USART_ASYNC::Flags::UsartStopBits sb = USART_ASYNC::Flags::OneStopBit,
						 typename USART_ASYNC::Flags::UsartParity p = USART_ASYNC::Flags::NoneParity,
						 typename USART_ASYNC::Flags::UsartMode m = USART_ASYNC::Flags::RxTxEnable) {
			USART_ASYNC::init(baudrate, db, sb, p, m);
			USART_ASYNC::setOnReceive(&_cbOnReceive);
		}

		static bool readByte(uint8_t *out) {
			return _inBuffer.pop(out);
		}
	};


#ifndef _ARM

	template<class HardwareUart>
	struct AdapterUsartSync_Debug {

		template<uint32_t BAUDRATE = 19200>
		AdapterUsartSync_Debug() {
			UsartSync<HardwareUart>::template init<BAUDRATE>();
		}

		void operator()(char c) const {
			while (!UsartSync<HardwareUart>::writeByte(c)) { continue; }
		}
	};

#endif


	template<class HardwareUart, uint8_t OUT_BUFFER_SIZE = 255>
	struct AdapterUsartAsync_Debug {
		template<uint32_t BAUDRATE = 19200>
		AdapterUsartAsync_Debug() {
			UsartAsync<HardwareUart, OUT_BUFFER_SIZE>::template init<BAUDRATE>();
		}

		void operator()(char c) const {
			UsartAsync<HardwareUart, OUT_BUFFER_SIZE>::writeByte(c);
		}
	};


	struct NullDebug {
		void operator()(char) const {}
	};


}

