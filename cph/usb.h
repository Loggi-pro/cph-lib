#pragma once
#include <void/static_assert.h>
#include <cph/containers/buffer.h>

#ifdef _STM8

#elif defined _AVR
	#include "platform/avr/usb/platform_usb.h"
#elif defined WIN32
	#include "platform/win32/usb/platform_usb.h" //simulation
#elif defined _ARM
	#include "platform/stm32/usb/platform_usb.h"
#endif

namespace cph {





template <TUsbNumber N = USB_DEFAULT>
class TUsb {
	private:
		typedef void(*cbOnReceive_t)(const uint8_t* values,uint32_t Len);
	public:
		typedef typename Private::UsbSelector<N>::selected Hardware;
        typedef typename Private::UsbSelector<N>::fakehelper Fake;

		static void init() {
			Hardware::init();
			
		}
		static void deinit() {
			Hardware::deinit();
			Hardware::setCallback(nullptr);
		}
		static void IntRxHandler() {
			if (!Hardware::IsRxInterrupt()) {return;}
		}
		static void IntTxEmptyHandler() {
			if (!Hardware::IsTxEmptyInterrupt()) {return;}
		}
		static void setCallback(cbOnReceive_t onDataReceive) {
				Hardware::setCallback(onDataReceive);
		}

		static bool writeData(uint8_t* data,uint32_t len){
			return Hardware::write(data,len);
		}
};

/*
void OnUSBReceiveData(uint8_t* data, uint32_t len) {
	cph::THardwareUsb0::writeData(data, len);
}
*/
}