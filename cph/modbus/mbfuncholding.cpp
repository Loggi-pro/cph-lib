/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbframe.h"
#include "mbconfig.h"
#include "mbutils.h"
/* ----------------------- Defines ------------------------------------------*/
#define MB_PDU_FUNC_READ_ADDR_OFF               ( MB_PDU_DATA_OFF + 0)
#define MB_PDU_FUNC_READ_REGCNT_OFF             ( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_FUNC_READ_SIZE                   ( 4 )
#define MB_PDU_FUNC_READ_REGCNT_MAX             ( 0x007D )

#define MB_PDU_FUNC_WRITE_ADDR_OFF              ( MB_PDU_DATA_OFF + 0)
#define MB_PDU_FUNC_WRITE_VALUE_OFF             ( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_FUNC_WRITE_SIZE                  ( 4 )

#define MB_PDU_FUNC_WRITE_MUL_ADDR_OFF          ( MB_PDU_DATA_OFF + 0 )
#define MB_PDU_FUNC_WRITE_MUL_REGCNT_OFF        ( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_FUNC_WRITE_MUL_BYTECNT_OFF       ( MB_PDU_DATA_OFF + 4 )
#define MB_PDU_FUNC_WRITE_MUL_VALUES_OFF        ( MB_PDU_DATA_OFF + 5 )
#define MB_PDU_FUNC_WRITE_MUL_SIZE_MIN          ( 5 )
#define MB_PDU_FUNC_WRITE_MUL_REGCNT_MAX        ( 0x0078 )

#define MB_PDU_FUNC_READWRITE_READ_ADDR_OFF     ( MB_PDU_DATA_OFF + 0 )
#define MB_PDU_FUNC_READWRITE_READ_REGCNT_OFF   ( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_FUNC_READWRITE_WRITE_ADDR_OFF    ( MB_PDU_DATA_OFF + 4 )
#define MB_PDU_FUNC_READWRITE_WRITE_REGCNT_OFF  ( MB_PDU_DATA_OFF + 6 )
#define MB_PDU_FUNC_READWRITE_BYTECNT_OFF       ( MB_PDU_DATA_OFF + 8 )
#define MB_PDU_FUNC_READWRITE_WRITE_VALUES_OFF  ( MB_PDU_DATA_OFF + 9 )
#define MB_PDU_FUNC_READWRITE_SIZE_MIN          ( 9 )

/* ----------------------- Static functions ---------------------------------*/
MBException    prveMBError2Exception( MBErrorCode errorCode );

/* ----------------------- Start implementation -----------------------------*/


MBException mbFuncWriteHoldingRegister(Modbus& obj, uint8_t* frame, uint16_t* length ) {
	#if MB_FUNC_WRITE_HOLDING_ENABLED > 0
	MBException status = MB_EX_NONE;

	if (*length != (MB_PDU_FUNC_WRITE_SIZE + MB_PDU_SIZE_MIN)) {
		/* Can't be a valid request because the length is incorrect. */
		return MB_EX_ILLEGAL_DATA_VALUE;
	}

	uint16_t regAddress = readAsU16_HighEndian(&frame[MB_PDU_FUNC_WRITE_ADDR_OFF]) + 1;
	/* Make callback to update the value. */
	MBErrorCode regStatus = obj.handlerRegHolding(&frame[MB_PDU_FUNC_WRITE_VALUE_OFF], regAddress, 1,
	                        MB_REG_WRITE);

	/* If an error occured convert it into a Modbus exception. */
	if (regStatus != MB_ENOERR) {
		status = prveMBError2Exception( regStatus );
	}

	return status;
	#else
	return MB_EX_ILLEGAL_FUNCTION;
	#endif
}

MBException mbFuncWriteMultipleHoldingRegister(Modbus& obj, uint8_t* frame, uint16_t* length ) {
	#if MB_FUNC_WRITE_MULTIPLE_HOLDING_ENABLED > 0
	MBException status = MB_EX_NONE;

	if (*length < (MB_PDU_FUNC_WRITE_MUL_SIZE_MIN + MB_PDU_SIZE_MIN)) {
		return 	MB_EX_ILLEGAL_DATA_VALUE;	/* Can't be a valid request because the length is incorrect. */
	}

	uint16_t regAddress = readAsU16_HighEndian(&frame[MB_PDU_FUNC_WRITE_MUL_ADDR_OFF]) + 1;
	uint16_t regCount = readAsU16_HighEndian(&frame[MB_PDU_FUNC_WRITE_MUL_REGCNT_OFF]);
	uint8_t regByteCount = frame[MB_PDU_FUNC_WRITE_MUL_BYTECNT_OFF];

	if ((regCount >= 1 ) && ( regCount <= MB_PDU_FUNC_WRITE_MUL_REGCNT_MAX) &&
	        (regByteCount == ( uint8_t ) ( 2 * regCount))) {
		/* Make callback to update the register values. */
		MBErrorCode regStatus = obj.handlerRegHolding( &frame[MB_PDU_FUNC_WRITE_MUL_VALUES_OFF],
		                        regAddress, regCount, MB_REG_WRITE );

		/* If an error occured convert it into a Modbus exception. */
		if ( regStatus != MB_ENOERR ) {
			status = prveMBError2Exception( regStatus );
		} else {
			/* The response contains the function code, the starting
			 * address and the quantity of registers. We reuse the
			 * old values in the buffer because they are still valid.
			 */
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

MBException mbFuncReadHoldingRegister(Modbus& obj, uint8_t* frame, uint16_t* length ) {
	#if MB_FUNC_READ_HOLDING_ENABLED > 0
	MBException    status = MB_EX_NONE;

	if (*length != (MB_PDU_FUNC_READ_SIZE + MB_PDU_SIZE_MIN)) {
		/* Can't be a valid request because the length is incorrect. */
		return MB_EX_ILLEGAL_DATA_VALUE;
	}

	uint16_t regAddress = readAsU16_HighEndian(&frame[MB_PDU_FUNC_READ_ADDR_OFF]) + 1;
	uint16_t regCount = readAsU16_HighEndian(&frame[MB_PDU_FUNC_READ_REGCNT_OFF]);

	/* Check if the number of registers to read is valid. If not
	 * return Modbus illegal data value exception.
	 */
	if ( ( regCount >= 1 ) && ( regCount <= MB_PDU_FUNC_READ_REGCNT_MAX ) ) {
		/* Set the current PDU data pointer to the beginning. */
		uint8_t*  frameCur = &frame[MB_PDU_FUNC_OFF];
		*length = MB_PDU_FUNC_OFF;
		/* First byte contains the function code. */
		*frameCur++ = MB_FUNC_READ_HOLDING_REGISTER;
		*length += 1;
		/* Second byte in the response contain the number of bytes. */
		*frameCur++ = ( uint8_t ) ( regCount * 2 );
		*length += 1;
		/* Make callback to fill the buffer. */
		MBErrorCode regStatus = obj.handlerRegHolding( frameCur, regAddress, regCount, MB_REG_READ );

		/* If an error occured convert it into a Modbus exception. */
		if ( regStatus != MB_ENOERR ) {
			status = prveMBError2Exception( regStatus );
		} else {
			*length += regCount * 2;
		}
	} else {
		status = MB_EX_ILLEGAL_DATA_VALUE;
	}

	return status;
	#else
	return MB_EX_ILLEGAL_FUNCTION;
	#endif
}

MBException mbFuncReadWriteMultipleHoldingRegister(Modbus& obj, uint8_t* frame,
        uint16_t* length ) {
	#if MB_FUNC_READWRITE_HOLDING_ENABLED > 0
	MBException    status = MB_EX_NONE;

	if (*length < (MB_PDU_FUNC_READWRITE_SIZE_MIN + MB_PDU_SIZE_MIN)) {
		return MB_EX_ILLEGAL_DATA_VALUE; /* Can't be a valid request because the length is incorrect. */
	}

	uint16_t regReadAddress = readAsU16_HighEndian(&frame[MB_PDU_FUNC_READWRITE_READ_ADDR_OFF]) + 1;
	uint16_t regReadCount = readAsU16_HighEndian(&frame[MB_PDU_FUNC_READWRITE_READ_REGCNT_OFF]);
	uint16_t regWriteAddress = readAsU16_HighEndian(&frame[MB_PDU_FUNC_READWRITE_WRITE_ADDR_OFF]) + 1;
	uint16_t regWriteCount = readAsU16_HighEndian(&frame[MB_PDU_FUNC_READWRITE_WRITE_REGCNT_OFF]);
	uint8_t regWriteByteCount = frame[MB_PDU_FUNC_READWRITE_BYTECNT_OFF];

	if ((regReadCount >= 1) && (regReadCount <= 0x7D) &&
	        (regWriteCount >= 1) && (regWriteCount <= 0x79) &&
	        ((2 * regWriteCount) == regWriteByteCount)) {
		/* Make callback to update the register values. */
		MBErrorCode regStatus = obj.handlerRegHolding(&frame[MB_PDU_FUNC_READWRITE_WRITE_VALUES_OFF],
		                        regWriteAddress, regWriteCount, MB_REG_WRITE);

		if (regStatus == MB_ENOERR) {
			/* Set the current PDU data pointer to the beginning. */
			uint8_t* frameCur = &frame[MB_PDU_FUNC_OFF];
			*length = MB_PDU_FUNC_OFF;
			/* First byte contains the function code. */
			*frameCur++ = MB_FUNC_READWRITE_MULTIPLE_REGISTERS;
			*length += 1;
			/* Second byte in the response contain the number of bytes. */
			*frameCur++ = ( uint8_t ) ( regReadCount * 2 );
			*length += 1;
			/* Make the read callback. */
			regStatus =
			    obj.handlerRegHolding( frameCur, regReadAddress, regReadCount, MB_REG_READ );

			if (regStatus == MB_ENOERR) {
				*length += 2 * regReadCount;
			}
		}

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
