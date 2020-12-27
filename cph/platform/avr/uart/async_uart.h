#pragma once
#include "platform_uart.h"
#include <cph/uart/selector.h>

namespace cph {
	template<u08 N = 0, uint8_t OUT_BUFFER_SIZE = 255>
	class TPlatformAsyncUsart {
			TPlatformAsyncUsart() {};
		public:
			typedef typename Private::UartSelector<N>::selected Hardware;
			typedef typename Hardware::Flags Flags;
			typedef typename Private::UartSelector<N>::fakehelper Spy;
		private:
			static cph::TBuffer<uint8_t, vd::Atomic> _buffer;
			static bool _cbTranceive(uint8_t* byte) {
				return _buffer.pop(byte);
			}
		public:


			template <uint32_t BAUDRATE,
			          typename Flags::UsartDataBits DB = Flags::DataBits8,
			          typename  Flags::UsartStopBits SB = Flags::OneStopBit,
			          typename  Flags::UsartParity P = Flags::NoneParity,
			          typename  Flags::UsartMode M = Flags::RxTxEnable
			          >
			static void init() {
				Hardware::template init<BAUDRATE, DB, SB, P, M>();
				_buffer.clear();
				Hardware::setCallback(nullptr, &_cbTranceive);
				Hardware::enableInterrupt(Hardware::Flags::RxNotEmptyInt);
			}

			static void init(uint32_t baudrate,
			                 typename  Flags::UsartDataBits db = Flags::DataBits8,
			                 typename Flags::UsartStopBits sb = Flags::OneStopBit,
			                 typename Flags::UsartParity p = Flags::NoneParity,
			                 typename Flags::UsartMode m = Flags::RxTxEnable) {
				Hardware::init(baudrate, db, sb, p, m);
				_buffer.clear();
				Hardware::setCallback(nullptr, &_cbTranceive);
				Hardware::enableInterrupt(Hardware::Flags::RxNotEmptyInt);
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

			static bool writeData(const vd::gsl::span<uint8_t>& data) {
				if (data.size() > (uint16_t)(_buffer.capacity() - _buffer.count())) { return false; }

				for (size_t i = 0; i < data.size(); i++) {
					uint8_t byte = data.data()[i];
					_buffer.push(byte);
				}

				Hardware::StartTranceive();
				return true;
			}

			static bool writeByte(uint8_t byte) {
				bool res  = _buffer.push(byte);

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
	template<u08 N, uint8_t OUT_BUFFER_SIZE>
	cph::TBuffer<uint8_t, vd::Atomic> TPlatformAsyncUsart<N, OUT_BUFFER_SIZE>::_buffer =
	    cph::TBuffer<uint8_t, vd::Atomic>
	    (cph::value_t<OUT_BUFFER_SIZE>());

}