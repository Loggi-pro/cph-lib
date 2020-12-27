
/* ----------------------- System includes ----------------------------------*/
#include "stdlib.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbframe.h"
#include "mbconfig.h"
#include "mbutils.h"
/* ----------------------- Defines ------------------------------------------*/
#define MB_PDU_FUNC_READ_ADDR_OFF           ( MB_PDU_DATA_OFF )
#define MB_PDU_FUNC_READ_COILCNT_OFF        ( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_FUNC_READ_SIZE               ( 4 )
#define MB_PDU_FUNC_READ_COILCNT_MAX        ( 0x07D0 )

#define MB_PDU_FUNC_WRITE_ADDR_OFF          ( MB_PDU_DATA_OFF )
#define MB_PDU_FUNC_WRITE_VALUE_OFF         ( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_FUNC_WRITE_SIZE              ( 4 )

#define MB_PDU_FUNC_WRITE_MUL_ADDR_OFF      ( MB_PDU_DATA_OFF )
#define MB_PDU_FUNC_WRITE_MUL_COILCNT_OFF   ( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_FUNC_WRITE_MUL_BYTECNT_OFF   ( MB_PDU_DATA_OFF + 4 )
#define MB_PDU_FUNC_WRITE_MUL_VALUES_OFF    ( MB_PDU_DATA_OFF + 5 )
#define MB_PDU_FUNC_WRITE_MUL_SIZE_MIN      ( 5 )
#define MB_PDU_FUNC_WRITE_MUL_COILCNT_MAX   ( 0x07B0 )

/* ----------------------- Static functions ---------------------------------*/
MBException    prveMBError2Exception( MBErrorCode errorCode );

/* ----------------------- Start implementation -----------------------------*/



MBException mbFuncReadCoils(Modbus& obj, uint8_t* frame, uint16_t* length) {
	#if MB_FUNC_READ_COILS_ENABLED > 0
	MBException status = MB_EX_NONE;

	if (*length != (MB_PDU_FUNC_READ_SIZE + MB_PDU_SIZE_MIN)) {
		/* Can't be a valid read coil register request because the length
		 * is incorrect. */
		return MB_EX_ILLEGAL_DATA_VALUE;
	}

	uint16_t regAddress = readAsU16_HighEndian(&frame[MB_PDU_FUNC_READ_ADDR_OFF]) + 1;
	uint16_t coilCount = readAsU16_HighEndian(&frame[MB_PDU_FUNC_READ_COILCNT_OFF]);

	/* Check if the number of registers to read is valid. If not
	 * return Modbus illegal data value exception.
	 */
	if ((coilCount >= 1) && (coilCount < MB_PDU_FUNC_READ_COILCNT_MAX)) {
		/* Set the current PDU data pointer to the beginning. */
		uint8_t* frameCur = &frame[MB_PDU_FUNC_OFF];
		*length = MB_PDU_FUNC_OFF;
		/* First byte contains the function code. */
		*frameCur++ = MB_FUNC_READ_COILS;
		*length += 1;
		/* Test if the quantity of coils is a multiple of 8. If not last
		 * byte is only partially field with unused coils set to zero. */
		uint8_t nBytes;

		if ( ( coilCount & 0x0007 ) != 0 ) {
			nBytes = ( uint8_t )( coilCount / 8 + 1 );
		} else {
			nBytes = ( uint8_t )( coilCount / 8 );
		}

		*frameCur++ = nBytes;
		*length += 1;
		MBErrorCode regStatus = obj.handlerRegCoils( frameCur, regAddress, coilCount,
		                        MB_REG_READ );

		/* If an error occured convert it into a Modbus exception. */
		if ( regStatus != MB_ENOERR ) {
			status = prveMBError2Exception( regStatus );
		} else {
			/* The response contains the function code, the starting address
			 * and the quantity of registers. We reuse the old values in the
			 * buffer because they are still valid. */
			*length += nBytes;;
		}
	} else {
		status = MB_EX_ILLEGAL_DATA_VALUE;
	}

	return status;
	#else
	return MB_EX_ILLEGAL_FUNCTION;
	#endif
}

MBException mbFuncWriteCoil(Modbus& obj, uint8_t* frame, uint16_t* length ) {
	#if MB_FUNC_WRITE_COIL_ENABLED > 0
	MBException status = MB_EX_NONE;

	if (*length != (MB_PDU_FUNC_WRITE_SIZE + MB_PDU_SIZE_MIN)) {
		/* Can't be a valid write coil register request because the length
		* is incorrect. */
		return MB_EX_ILLEGAL_DATA_VALUE;
	}

	uint16_t regAddress = readAsU16_HighEndian(&frame[MB_PDU_FUNC_WRITE_ADDR_OFF]) + 1;

	if ((frame[MB_PDU_FUNC_WRITE_VALUE_OFF + 1] == 0x00)
	        &&  ((frame[MB_PDU_FUNC_WRITE_VALUE_OFF] == 0xFF) ||
	             (frame[MB_PDU_FUNC_WRITE_VALUE_OFF] == 0x00))) {
		uint8_t buf[2] = { 0 };

		if (frame[MB_PDU_FUNC_WRITE_VALUE_OFF] == 0xFF) {
			buf[0] = 1;
		}

		MBErrorCode regStatus = obj.handlerRegCoils( &buf[0], regAddress, 1, MB_REG_WRITE );

		/* If an error occured convert it into a Modbus exception. */
		if (regStatus != MB_ENOERR) {
			status = prveMBError2Exception(regStatus);
		}
	} else {
		status = MB_EX_ILLEGAL_DATA_VALUE;
	}

	return status;
	#else
	return MB_EX_ILLEGAL_FUNCTION;
	#endif
}

MBException mbFuncWriteMultipleCoils(Modbus& obj, uint8_t* frame, uint16_t* length ) {
	#if MB_FUNC_WRITE_MULTIPLE_COILS_ENABLED > 0
	MBException status = MB_EX_NONE;

	if (*length <= (MB_PDU_FUNC_WRITE_SIZE + MB_PDU_SIZE_MIN)) {
		/* Can't be a valid write coil register request because the length
		* is incorrect. */
		return MB_EX_ILLEGAL_DATA_VALUE;
	}

	uint16_t regAddress = readAsU16_HighEndian(&frame[MB_PDU_FUNC_WRITE_MUL_ADDR_OFF]) + 1;
	uint16_t coilCount = readAsU16_HighEndian(&frame[MB_PDU_FUNC_WRITE_MUL_COILCNT_OFF]) + 1;
	uint8_t byteCount = frame[MB_PDU_FUNC_WRITE_MUL_BYTECNT_OFF];
	uint8_t byteCountVerify;

	/* Compute the number of expected bytes in the request. */
	if ( ( coilCount & 0x0007 ) != 0 ) {
		byteCountVerify = ( uint8_t )( coilCount / 8 + 1 );
	} else {
		byteCountVerify = ( uint8_t )( coilCount / 8 );
	}

	if ((coilCount >= 1) && (coilCount <= MB_PDU_FUNC_WRITE_MUL_COILCNT_MAX)
	        && (byteCountVerify == byteCount)) {
		MBErrorCode regStatus = obj.handlerRegCoils( &frame[MB_PDU_FUNC_WRITE_MUL_VALUES_OFF],
		                        regAddress, coilCount, MB_REG_WRITE );

		/* If an error occured convert it into a Modbus exception. */
		if ( regStatus != MB_ENOERR ) {
			status = prveMBError2Exception( regStatus );
		} else {
			/* The response contains the function code, the starting address
			 * and the quantity of registers. We reuse the old values in the
			 * buffer because they are still valid. */
			*length = MB_PDU_FUNC_WRITE_MUL_BYTECNT_OFF;
		}
	} else {
		status = MB_EX_ILLEGAL_DATA_VALUE;
	}

	return status;
	#else
	return MB_EX_ILLEGAL_FUNCTION;
	#endif
}


