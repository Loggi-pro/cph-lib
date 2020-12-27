#pragma once
#include <void\static_assert.h>
#include <void\assert.h>
#include <cph\containers\buffer.h>
#include <cph/platform/fake/usb/usb_fake.h>
#include <usbd_def.h>
#include "usb_desc.h"
#include "usb_interface.h"
#include <void/delay.h>
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

	namespace Private {

		struct Usb0Regs {
			private:
				static USBD_HandleTypeDef  _hUsbDeviceFS;
			public:
				typedef  TUsbCDCInterface<&_hUsbDeviceFS, 512, 512> InterfaceClass;
				static USBD_HandleTypeDef* getHandle() {
					return &_hUsbDeviceFS;
				}
				static USBD_DescriptorsTypeDef* getDescriptor() {
					static const TDescriptorConfig cfg = {"Loggi PRO Virtual ComPort",
					                                      "Loggi PRO", "CDC Config", "CDC Interface"
					                                     };
					return TUsbDescriptor<&_hUsbDeviceFS, cfg>::getDescriptor();
				}
				static USBD_CDC_ItfTypeDef* getCDCInterface() {
					return InterfaceClass::getInterface();
				}
		};

		template <class UsbRegs>
		struct UsbFlags {};

		template<>
		struct UsbFlags<Usb0Regs> {
			enum Error {
				NoError = 0
			};
		};


	}
	template <class UsbRegs>
	struct Usb0RegsActions {
		private:
			static inline bool _isInited = false;
			static void _USB_Init() {
				/* Unlink previous class*/
				UsbRegs::getHandle()->pClass = NULL;
				/* Assign USBD Descriptors */
				UsbRegs::getHandle()->pDesc = UsbRegs::getDescriptor();
				/* Set Device initial State */
				UsbRegs::getHandle()->dev_state  = USBD_STATE_DEFAULT;
				UsbRegs::getHandle()->id = DEVICE_FS;
				USBD_LL_Init(UsbRegs::getHandle());
				//HAL_PCD_DevDisconnect((PCD_HandleTypeDef*)(UsbRegs::getHandle()->pData));
				//HAL_PCD_DevConnect((PCD_HandleTypeDef*)(UsbRegs::getHandle()->pData));
			}
			static void _USB_DeInit() {
				UsbRegs::getHandle()->dev_state  = USBD_STATE_DEFAULT;
				/* Free Class Resources */
				UsbRegs::getHandle()->pClass->DeInit(UsbRegs::getHandle(), UsbRegs::getHandle()->dev_config);
				/* Stop the low level driver  */
				USBD_LL_Stop(UsbRegs::getHandle());
				/* Initialize low level driver */
				USBD_LL_DeInit(UsbRegs::getHandle()); //deinit gpio
			}
			static void _USB_RegisterClass() {
				/* link the class to the USB Device handle */
				UsbRegs::getHandle()->pClass = &USBD_CDC;
			}
			static void _USB_RegisterInterface() {
				auto status = USBD_CDC_RegisterInterface(UsbRegs::getHandle(), UsbRegs::getCDCInterface());
				assert(status == USBD_OK);
			}

			static void _USB_Start() {
				USBD_LL_Start(UsbRegs::getHandle());
			}
			static void _USB_Stop() {
				UsbRegs::getHandle()->pClass->DeInit(UsbRegs::getHandle(), UsbRegs::getHandle()->dev_config);
				/* Stop the low level driver  */
				USBD_LL_Stop(UsbRegs::getHandle());
			}
		public:
			typedef UsbRegs Regs;
			typedef Private::UsbFlags<UsbRegs> Flags;

			static bool writeData(const uint8_t* data, uint32_t len) {
				if (!_isInited) { return true; }

				return Regs::InterfaceClass::transmitData(data, len);
			}
			static void deinit() {
				_USB_DeInit();
				__HAL_RCC_USB_CLK_DISABLE(); /* Peripheral clock disable */
				/**USB GPIO Configuration
				PA11     ------> USB_DM
				PA12     ------> USB_DP
				*/
				const uint16_t DM_Pin = GPIO_PIN_11;
				const uint16_t DP_Pin = GPIO_PIN_12;
				GPIO_TypeDef* const DM_GPIO_Port = GPIOA;
				HAL_GPIO_DeInit(DM_GPIO_Port, DM_Pin | DP_Pin);
				HAL_NVIC_DisableIRQ(USB_LP_CAN_RX0_IRQn);
				_isInited = false;
			}
			static void init() {
				//init clock
				RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
				HAL_RCCEx_GetPeriphCLKConfig(&PeriphClkInit);
				PeriphClkInit.USBClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5; //72MHZ/1.5
				assert(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) == HAL_OK);
				//init gpio
				GPIO_InitTypeDef GPIO_InitStruct = {0};
				__HAL_RCC_GPIOA_CLK_ENABLE();
				//USB GPIO Configuration
				//PA11     ------> USB_DM
				//PA12     ------> USB_DP
				const uint16_t DM_Pin = GPIO_PIN_11;
				const uint16_t DP_Pin = GPIO_PIN_12;
				GPIO_TypeDef* const DM_GPIO_Port = GPIOA;
				//D+ to 0, to reset USB
				GPIO_InitStruct.Pin = DP_Pin;
				GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
				GPIO_InitStruct.Pull = GPIO_NOPULL;
				GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
				HAL_GPIO_Init(DM_GPIO_Port, &GPIO_InitStruct);
				HAL_GPIO_WritePin(DM_GPIO_Port, DP_Pin, GPIO_PIN_RESET);
				vd::delay(5_ms);
				//GPIO_TypeDef*const  DP_GPIO_Port = GPIOA;
				GPIO_InitStruct.Pin = DM_Pin | DP_Pin;
				GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
				GPIO_InitStruct.Pull = GPIO_NOPULL;
				GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
				GPIO_InitStruct.Alternate = GPIO_AF14_USB;
				__HAL_RCC_USB_CLK_ENABLE(); // USB clock enable
				HAL_GPIO_Init(DM_GPIO_Port, &GPIO_InitStruct);
				// Peripheral interrupt init
				HAL_NVIC_SetPriority(USB_LP_CAN_RX0_IRQn, 0, 0);
				_USB_Init();
				_USB_RegisterClass();
				_USB_RegisterInterface();
				HAL_NVIC_EnableIRQ(USB_LP_CAN_RX0_IRQn);
				start();
				_isInited = true;
			}
			static void start() {
				_USB_Start();
			}
			static void stop() {
				_USB_Stop();
			}
			static void enableInterrupt() {
				HAL_NVIC_EnableIRQ(USB_LP_CAN_RX0_IRQn);
			}

			static void disableInterrupt() {
				HAL_NVIC_DisableIRQ(USB_LP_CAN_RX0_IRQn);
			}

			static void setCallback(typename Regs::InterfaceClass::cbOnReceive_t onDataReceive) {
				Regs::InterfaceClass::setCallback(onDataReceive);
			}
	};


	typedef Usb0RegsActions<Private::Usb0Regs> THardwareUsb0;

	typedef struct {} UsbFakeHelperStub;

	namespace Private {
		template <TUsbNumber n>
		struct UsbSelector {};

		template <>
		struct UsbSelector <USB_FAKE> {
			typedef TFakeUsb selected;
			typedef UsbFakeHelper fakehelper;

		};
		#ifdef CPH_HAS_USB0
		template <>
		struct UsbSelector <USB_0> {
			typedef THardwareUsb0 selected;
			typedef UsbFakeHelperStub fakehelper;

		};
		#endif
		#ifdef CPH_HAS_USB1
		template<>
		struct UsbSelector<USB_1> {
			typedef THardwareUsb1 selected;
			typedef UsbFakeHelperStub fakehelper;
		};
		#endif
	}


}

