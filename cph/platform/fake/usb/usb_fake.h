#pragma once
#include <void\static_assert.h>
#include <void\assert.h>
#include <cph\containers\buffer.h>
namespace cph {

/*#ifndef CPH_HAS_UART0
	#define CPH_HAS_UART0		(true)
	#define CPH_IT_USART_RXC      0
	#define CPH_IT_USART_TXE    1
	#define CPH_IT_USART_TXC   1

#endif*/




struct UsbFakeFlags {
	
	enum Error {
		NoError = 0,
		/*OverrunError = 1 << 0,
		NoiseError = 1 << 2,
		FramingError = 1 << 3,
		ParityError = 1 << 4,
		ErrorMask = OverrunError | FramingError | ParityError*/
	};
};


template <uint8_t BUFFER_SIZE>
struct UsbFakeRegsActions {
	private:
		friend struct UsbFakeHelper; //fake functions
		static UsbFakeFlags::Error _error;
		static bool _isInited;
		static TBuffer<uint8_t> _outBuffer;
		static TBuffer<uint8_t> _rxBuffer;
	public:

		typedef UsbFakeFlags Flags;

		/*static bool writeReady() {
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
		}*/


		static void deinit() {
			_isInited = false;
		}

		static void init() {
			_isInited = true;

		}

		/*static void enableInterrupt(typename UsartFakeFlags::InterruptFlags interrupts) {
		}

		static void disableInterrupt(typename UsartFakeFlags::InterruptFlags interrupts) {
		}*/
		static bool IsRxInterrupt() {
			return true;
		}
		static bool IsTxEmptyInterrupt() {
			return true;
		}

		static bool IsTxCompleteInterrupt() {
			return true;
		}

		static typename UsbFakeFlags::Error getError() {
			typename UsbFakeFlags::Error tmp = _error;
			_error = Flags::NoError;
			return tmp;
		}

};
template < uint8_t BUFFER_SIZE>
UsbFakeFlags::Error UsbFakeRegsActions<BUFFER_SIZE>::_error = UsbFakeFlags::NoError;

template < uint8_t BUFFER_SIZE>
bool UsbFakeRegsActions<BUFFER_SIZE>::_isInited = false;

template < uint8_t BUFFER_SIZE>
TBuffer<uint8_t> UsbFakeRegsActions<BUFFER_SIZE>::_rxBuffer =
    TBuffer<uint8_t>{cph::value_t<BUFFER_SIZE>()};

template < uint8_t BUFFER_SIZE>
TBuffer<uint8_t> UsbFakeRegsActions< BUFFER_SIZE>::_outBuffer =
    TBuffer<uint8_t>{cph::value_t<BUFFER_SIZE>()};

typedef UsbFakeRegsActions <255> TFakeUsb;

//fake helper functions
struct UsbFakeHelper {

	static void setError(typename UsbFakeFlags::Error err) {
		TFakeUsb::_error = err;
	}
	static bool isInited() {
		return TFakeUsb::_isInited;
	}
	static bool setReceiveByte(uint8_t data) {
		bool result = TFakeUsb::_rxBuffer.push(data);
		assert(result); //TEST BUFFER OVERFLOW -> CHANGE SIZE
		return result;
	}
	static void clearOutBuffer(void) {
		TFakeUsb::_outBuffer.clear();
	}
	static bool getSendedByte(u08* data) {
		return TFakeUsb::_outBuffer.pop(data);
	}
};





}

