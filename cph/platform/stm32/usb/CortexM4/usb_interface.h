#pragma once
#include <usbd_cdc.h>
#include <usbd_def.h>
#include <stdint.h>
#include <void/assert.h>
template <USBD_HandleTypeDef* USB_HANDLE,uint16_t APP_RX_DATA_SIZE,uint16_t APP_TX_DATA_SIZE>
class TUsbCDCInterface {
	public:
		typedef void(*cbOnReceive_t)(uint8_t* values, uint32_t Len);
	private:
		static volatile cbOnReceive_t _onDataReceiveCallback;
		static uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];
		static uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];
		static int8_t CDC_Init_FS(void);
		static int8_t CDC_DeInit_FS(void);
		static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
		static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t* Len);


	public:


		static USBD_CDC_ItfTypeDef* getInterface() {
			static USBD_CDC_ItfTypeDef _ifHandle = {
				CDC_Init_FS,
				CDC_DeInit_FS,
				CDC_Control_FS,
				CDC_Receive_FS
			};
			return &_ifHandle;
		}

		static bool transmitData(const uint8_t* Buf, uint32_t Len);

		static void setCallback(cbOnReceive_t onDataReceive) {
			_onDataReceiveCallback = onDataReceive;
		}
};
template <USBD_HandleTypeDef* USB_HANDLE,uint16_t APP_RX_DATA_SIZE,uint16_t APP_TX_DATA_SIZE>
typename TUsbCDCInterface<USB_HANDLE,APP_RX_DATA_SIZE,APP_TX_DATA_SIZE>::cbOnReceive_t volatile
TUsbCDCInterface<USB_HANDLE,APP_RX_DATA_SIZE,APP_TX_DATA_SIZE>::_onDataReceiveCallback = nullptr;

template <USBD_HandleTypeDef* USB_HANDLE,uint16_t APP_RX_DATA_SIZE,uint16_t APP_TX_DATA_SIZE>
uint8_t TUsbCDCInterface<USB_HANDLE,APP_RX_DATA_SIZE,APP_TX_DATA_SIZE>::UserRxBufferFS[APP_RX_DATA_SIZE];
template <USBD_HandleTypeDef* USB_HANDLE,uint16_t APP_RX_DATA_SIZE,uint16_t APP_TX_DATA_SIZE>
uint8_t TUsbCDCInterface<USB_HANDLE,APP_RX_DATA_SIZE,APP_TX_DATA_SIZE>::UserTxBufferFS[APP_TX_DATA_SIZE];

template <USBD_HandleTypeDef* USB_HANDLE,uint16_t APP_RX_DATA_SIZE,uint16_t APP_TX_DATA_SIZE>
int8_t TUsbCDCInterface<USB_HANDLE,APP_RX_DATA_SIZE,APP_TX_DATA_SIZE>::CDC_Init_FS(void) {
	USBD_CDC_SetTxBuffer(USB_HANDLE, UserTxBufferFS, 0);
	USBD_CDC_SetRxBuffer(USB_HANDLE, UserRxBufferFS);
	return (USBD_OK);
}


/**
  * @brief  DeInitializes the CDC media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
template <USBD_HandleTypeDef* USB_HANDLE,uint16_t APP_RX_DATA_SIZE,uint16_t APP_TX_DATA_SIZE>
int8_t TUsbCDCInterface<USB_HANDLE,APP_RX_DATA_SIZE,APP_TX_DATA_SIZE>::CDC_DeInit_FS(void) {
	return (USBD_OK);
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
template <USBD_HandleTypeDef* USB_HANDLE,uint16_t APP_RX_DATA_SIZE,uint16_t APP_TX_DATA_SIZE>
int8_t TUsbCDCInterface<USB_HANDLE,APP_RX_DATA_SIZE,APP_TX_DATA_SIZE>::CDC_Control_FS(uint8_t cmd, uint8_t* pbuf,
        uint16_t length) {
	switch (cmd) {
	case CDC_SEND_ENCAPSULATED_COMMAND:
		break;

	case CDC_GET_ENCAPSULATED_RESPONSE:
		break;

	case CDC_SET_COMM_FEATURE:
		break;

	case CDC_GET_COMM_FEATURE:
		break;

	case CDC_CLEAR_COMM_FEATURE:
		break;

	/*******************************************************************************/
	/* Line Coding Structure                                                       */
	/*-----------------------------------------------------------------------------*/
	/* Offset | Field       | Size | Value  | Description                          */
	/* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
	/* 4      | bCharFormat |   1  | Number | Stop bits                            */
	/*                                        0 - 1 Stop bit                       */
	/*                                        1 - 1.5 Stop bits                    */
	/*                                        2 - 2 Stop bits                      */
	/* 5      | bParityType |  1   | Number | Parity                               */
	/*                                        0 - None                             */
	/*                                        1 - Odd                              */
	/*                                        2 - Even                             */
	/*                                        3 - Mark                             */
	/*                                        4 - Space                            */
	/* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
	/*******************************************************************************/
	case CDC_SET_LINE_CODING:
		break;

	case CDC_GET_LINE_CODING:
		break;

	case CDC_SET_CONTROL_LINE_STATE:
		break;

	case CDC_SEND_BREAK:
		break;

	default:
		break;
	}

	return (USBD_OK);
	/* USER CODE END 5 */
}

/**
  * @brief  Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will block any OUT packet reception on USB endpoint
  *         untill exiting this function. If you exit this function before transfer
  *         is complete on CDC interface (ie. using DMA controller) it will result
  *         in receiving more data while previous ones are still not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
template <USBD_HandleTypeDef* USB_HANDLE,uint16_t APP_RX_DATA_SIZE,uint16_t APP_TX_DATA_SIZE>
int8_t TUsbCDCInterface<USB_HANDLE,APP_RX_DATA_SIZE,APP_TX_DATA_SIZE>::CDC_Receive_FS(uint8_t* Buf, uint32_t* Len) {
	if (_onDataReceiveCallback != nullptr) { _onDataReceiveCallback(Buf, *Len); }

	//USBD_CDC_SetRxBuffer(USB_HANDLE, &Buf[0]);



	USBD_CDC_ReceivePacket(USB_HANDLE);
	return (USBD_OK);
}

/**
  * @brief  CDC_Transmit_FS
  *         Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  *         @note
  *
  *
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
template <USBD_HandleTypeDef* USB_HANDLE,uint16_t APP_RX_DATA_SIZE,uint16_t APP_TX_DATA_SIZE>
bool TUsbCDCInterface<USB_HANDLE,APP_RX_DATA_SIZE,APP_TX_DATA_SIZE>::transmitData(const uint8_t* Buf, uint32_t Len) {
	USBD_StatusTypeDef result = USBD_OK;
	USBD_CDC_HandleTypeDef* hcdc = (USBD_CDC_HandleTypeDef*)USB_HANDLE->pClassData;

	if (hcdc->TxState != 0) {
		return false;
	}

	memcpy(UserTxBufferFS, Buf, Len);
	assert(Len <= APP_TX_DATA_SIZE);
	USBD_CDC_SetTxBuffer(USB_HANDLE, UserTxBufferFS, Len);
	result = (USBD_StatusTypeDef)USBD_CDC_TransmitPacket(USB_HANDLE);
	return result == USBD_OK;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
