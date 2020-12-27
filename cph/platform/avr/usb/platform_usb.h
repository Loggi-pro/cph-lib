#pragma once

namespace cph {

#define CPH_HAS_USB0

enum TUsbNumber {
	USB_FAKE = -1
	#ifdef CPH_HAS_USB0
	, USB_0 = 0
	 #endif
	#ifdef CPH_HAS_USB1
	, USB_1 = 1
	#endif
	, USB_DEFAULT = USB_0
};

struct UsbRegsActions {
	private:
	public:
		typedef UsbRegs Regs;
		typedef Private::UsbFlags<UsbRegs> Flags;

		static bool writeData(const uint8_t* data, uint32_t len) {
			
		}
		static void deinit() {

		}
		static void init() {
			start();
		}
		static void start() {			
			
		}
		static void stop() {
			
		}
		static void enableInterrupt() {
			
		}

		static void disableInterrupt() {
			
		}

		static void setCallback(typename Regs::InterfaceClass::cbOnReceive_t onDataReceive) {
			
		}
};


typedef UsbRegsActions TSoftwareUsb0;



#ifdef CPH_HAS_USB0
template <>
struct UsbSelector <USB_0> {
	typedef TSoftwareUsb0 selected;
	typedef UsbFakeHelperStub fakehelper;

};
#endif
	
}