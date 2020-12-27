#pragma once
#include <usbd_def.h>
#include <usbd_core.h>


#define  USB_SIZ_STRING_SERIAL       0x1A

struct TDescriptorConfig {
	const char*  USBD_PRODUCT_STRING_FS;// "STM32 Virtual ComPort"
	const char*  USBD_MANUFACTURER_STRING; //"STMicroelectronics"
	const char*  USBD_CONFIGURATION_STRING_FS;// "CDC Config"
	const char*  USBD_INTERFACE_STRING_FS; // "CDC Interface"
};

template <USBD_HandleTypeDef* USB_HANDLE, TDescriptorConfig const& cfg>
class TUsbDescriptor {
	private:
		static void Get_SerialNum(void);


		static void IntToUnicode(uint32_t value, uint8_t* pbuf, uint8_t len);
		static uint8_t* USBD_FS_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t* length);
		static uint8_t* USBD_FS_LangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t* length);
		static uint8_t* USBD_FS_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t* length);
		static uint8_t* USBD_FS_ProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t* length);
		static uint8_t* USBD_FS_SerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t* length);
		static uint8_t* USBD_FS_ConfigStrDescriptor(USBD_SpeedTypeDef speed, uint16_t* length);
		static uint8_t* USBD_FS_InterfaceStrDescriptor(USBD_SpeedTypeDef speed, uint16_t* length);

	public:
		static USBD_DescriptorsTypeDef* getDescriptor() {
			static USBD_DescriptorsTypeDef _FS_Desc = {
				USBD_FS_DeviceDescriptor
				, USBD_FS_LangIDStrDescriptor
				, USBD_FS_ManufacturerStrDescriptor
				, USBD_FS_ProductStrDescriptor
				, USBD_FS_SerialStrDescriptor
				, USBD_FS_ConfigStrDescriptor
				, USBD_FS_InterfaceStrDescriptor
			};
			return &_FS_Desc;
		}
};
extern __ALIGN_BEGIN uint8_t USBD_FS_DeviceDesc[USB_LEN_DEV_DESC] __ALIGN_END;
extern __ALIGN_BEGIN uint8_t USBD_LangIDDesc[USB_LEN_LANGID_STR_DESC] __ALIGN_END;
extern __ALIGN_BEGIN uint8_t USBD_StrDesc[USBD_MAX_STR_DESC_SIZ] __ALIGN_END;
extern __ALIGN_BEGIN uint8_t USBD_StringSerial[USB_SIZ_STRING_SERIAL] __ALIGN_END;
/**
  * @brief  Create the serial number string descriptor
  * @param  None
  * @retval None
  */
template <USBD_HandleTypeDef* USB_HANDLE, TDescriptorConfig const& cfg>
void TUsbDescriptor<USB_HANDLE, cfg>::Get_SerialNum(void) {
	const uint32_t DEVICE_ID1 = (UID_BASE);
	const uint32_t DEVICE_ID2 = (UID_BASE + 0x4);
	const uint32_t DEVICE_ID3 = (UID_BASE + 0x8);
	const uint32_t deviceserial0 = *(uint32_t*) DEVICE_ID1 + (*(uint32_t*) DEVICE_ID3);
	const uint32_t deviceserial1 = *(uint32_t*) DEVICE_ID2;

	if (deviceserial0 != 0) {
		IntToUnicode(deviceserial0, &USBD_StringSerial[2], 8);
		IntToUnicode(deviceserial1, &USBD_StringSerial[18], 4);
	}
}

		/**
		* @brief  Convert Hex 32Bits value into char
		* @param  value: value to convert
		* @param  pbuf: pointer to the buffer
		* @param  len: buffer length
		* @retval None
		*/
template <USBD_HandleTypeDef* USB_HANDLE, TDescriptorConfig const& cfg>
void TUsbDescriptor<USB_HANDLE, cfg>::IntToUnicode(uint32_t value, uint8_t* pbuf, uint8_t len) {
	uint8_t idx = 0;

	for (idx = 0; idx < len; idx++) {
		if (((value >> 28)) < 0xA) {
			pbuf[2 * idx] = (value >> 28) + '0';
		} else {
			pbuf[2 * idx] = (value >> 28) + 'A' - 10;
		}

		value = value << 4;
		pbuf[2 * idx + 1] = 0;
	}
}



/**
  * @brief  Return the device descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
template <USBD_HandleTypeDef* USB_HANDLE, TDescriptorConfig const& cfg>
uint8_t* TUsbDescriptor<USB_HANDLE, cfg>::USBD_FS_DeviceDescriptor(USBD_SpeedTypeDef speed,
        uint16_t* length) {
	*length = sizeof(USBD_FS_DeviceDesc);
	return USBD_FS_DeviceDesc;
}


/**
  * @brief  Return the LangID string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
template <USBD_HandleTypeDef* USB_HANDLE, TDescriptorConfig const& cfg>
uint8_t* TUsbDescriptor<USB_HANDLE, cfg>::USBD_FS_LangIDStrDescriptor(USBD_SpeedTypeDef speed,
        uint16_t* length) {
	*length = sizeof(USBD_LangIDDesc);
	return USBD_LangIDDesc;
}

/**
  * @brief  Return the product string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
template <USBD_HandleTypeDef* USB_HANDLE, TDescriptorConfig const& cfg>
uint8_t* TUsbDescriptor<USB_HANDLE, cfg>::USBD_FS_ProductStrDescriptor(USBD_SpeedTypeDef speed,
        uint16_t* length) {
	if (speed == 0) {
		USBD_GetString((uint8_t*)cfg.USBD_PRODUCT_STRING_FS, USBD_StrDesc, length);
	} else {
		USBD_GetString((uint8_t*)cfg.USBD_PRODUCT_STRING_FS, USBD_StrDesc, length);
	}

	return USBD_StrDesc;
}

/**
  * @brief  Return the manufacturer string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
template <USBD_HandleTypeDef* USB_HANDLE, TDescriptorConfig const& cfg>
uint8_t* TUsbDescriptor<USB_HANDLE, cfg>::USBD_FS_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed,
        uint16_t* length) {
	USBD_GetString((uint8_t*)cfg.USBD_MANUFACTURER_STRING, USBD_StrDesc, length);
	return USBD_StrDesc;
}

/**
  * @brief  Return the serial number string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
template <USBD_HandleTypeDef* USB_HANDLE, TDescriptorConfig const& cfg>
uint8_t* TUsbDescriptor<USB_HANDLE, cfg>::USBD_FS_SerialStrDescriptor(USBD_SpeedTypeDef speed,
        uint16_t* length) {
	*length = USB_SIZ_STRING_SERIAL;
	/* Update the serial number string descriptor with the data from the unique
	 * ID */
	Get_SerialNum();
	return (uint8_t*) USBD_StringSerial;
}

/**
  * @brief  Return the configuration string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
template <USBD_HandleTypeDef* USB_HANDLE, TDescriptorConfig const& cfg>
uint8_t* TUsbDescriptor<USB_HANDLE, cfg>::USBD_FS_ConfigStrDescriptor(USBD_SpeedTypeDef speed,
        uint16_t* length) {
	if (speed == USBD_SPEED_HIGH) {
		USBD_GetString((uint8_t*)cfg.USBD_CONFIGURATION_STRING_FS, USBD_StrDesc, length);
	} else {
		USBD_GetString((uint8_t*)cfg.USBD_CONFIGURATION_STRING_FS, USBD_StrDesc, length);
	}

	return USBD_StrDesc;
}

/**
  * @brief  Return the interface string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
template <USBD_HandleTypeDef* USB_HANDLE, TDescriptorConfig const& cfg>
uint8_t* TUsbDescriptor<USB_HANDLE, cfg>::USBD_FS_InterfaceStrDescriptor(USBD_SpeedTypeDef speed,
        uint16_t* length) {
	if (speed == 0) {
		USBD_GetString((uint8_t*)cfg.USBD_INTERFACE_STRING_FS, USBD_StrDesc, length);
	} else {
		USBD_GetString((uint8_t*)cfg.USBD_INTERFACE_STRING_FS, USBD_StrDesc, length);
	}

	return USBD_StrDesc;
}


