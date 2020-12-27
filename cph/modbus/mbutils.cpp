
#include "mbtypes.h"
/* ----------------------- Start implementation -----------------------------*/

MBException prveMBError2Exception( MBErrorCode eErrorCode ) {
	MBException    status;

	switch ( eErrorCode ) {
	case MB_ENOERR:
		status = MB_EX_NONE;
		break;

	case MB_ENOREG:
		status = MB_EX_ILLEGAL_DATA_ADDRESS;
		break;

	case MB_ETIMEDOUT:
		status = MB_EX_SLAVE_BUSY;
		break;

	default:
		status = MB_EX_SLAVE_DEVICE_FAILURE;
		break;
	}

	return status;
}

/* ----------------------- Platform includes --------------------------------*/
//https://electronix.ru/forum/index.php?app=forums&module=forums&controller=topic&id=53780
template <uint16_t POLYNOM>
uint16_t crc16(uint16_t varCrc, uint8_t data) {
	bool flag = false;
	uint16_t result = varCrc ^= data;

	for (uint8_t i = 0; i < 8; ++i) {
		flag = (result & 1) > 0;
		result >>= 1;

		if (flag) { result ^= POLYNOM; }
	}

	return result;
}

uint16_t calc_crc16(uint8_t* arr, uint16_t len) {
	uint16_t result = 0xFFFF; //init data;

	for (uint8_t i = 0; i < len; i++) {
		result = crc16<0xA001>(result, *arr++);
	}

	return result;
}

uint16_t readAsU16_HighEndian(uint8_t* arr) {
	uint16_t result = (uint16_t)(arr[0] << 8);
	result |= (uint16_t)(arr[1]);
	return result;
}