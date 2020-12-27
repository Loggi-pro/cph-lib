
/* ----------------------- System includes ----------------------------------*/
#include "stdlib.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbframe.h"
#include "mbconfig.h"
#include "mbutils.h"
/* ----------------------- Defines ------------------------------------------*/
#define MB_PDU_FUNC_READ_ADDR_OFF           ( MB_PDU_DATA_OFF )
#define MB_PDU_FUNC_READ_DISCCNT_OFF        ( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_FUNC_READ_SIZE               ( 4 )
#define MB_PDU_FUNC_READ_DISCCNT_MAX        ( 0x07D0 )

/* ----------------------- Static functions ---------------------------------*/
MBException    prveMBError2Exception( MBErrorCode errorCode );

/* ----------------------- Start implementation -----------------------------*/



MBException mbFuncReadDiscreteInputs(Modbus& obj, uint8_t* frame, uint16_t* length ) {
	#if MB_FUNC_READ_COILS_ENABLED > 0
	MBException status = MB_EX_NONE;

	if (*length != (MB_PDU_FUNC_READ_SIZE + MB_PDU_SIZE_MIN)) {
		/* Can't be a valid read coil register request because the length
		* is incorrect. */
		return MB_EX_ILLEGAL_DATA_VALUE;
	}

	uint16_t regAddress = readAsU16_HighEndian(&frame[MB_PDU_FUNC_READ_ADDR_OFF]) + 1;
	uint16_t discreteCount = readAsU16_HighEndian(&frame[MB_PDU_FUNC_READ_DISCCNT_OFF]);

	/* Check if the number of registers to read is valid. If not
	 * return Modbus illegal data value exception.
	 */
	if ((discreteCount >= 1 ) && (discreteCount < MB_PDU_FUNC_READ_DISCCNT_MAX)) {
		/* Set the current PDU data pointer to the beginning. */
		uint8_t* frameCur = &frame[MB_PDU_FUNC_OFF];
		*length = MB_PDU_FUNC_OFF;
		/* First byte contains the function code. */
		*frameCur++ = MB_FUNC_READ_DISCRETE_INPUTS;
		*length += 1;
		/* Test if the quantity of coils is a multiple of 8. If not last
		 * byte is only partially field with unused coils set to zero. */
		uint8_t  nBytes;

		if ((discreteCount & 0x0007 ) != 0) {
			nBytes = ( uint8_t ) ( discreteCount / 8 + 1 );
		} else {
			nBytes = ( uint8_t ) ( discreteCount / 8 );
		}

		*frameCur++ = nBytes;
		*length += 1;
		MBErrorCode regStatus =
		    obj.handlerRegDiscrete( frameCur, regAddress, discreteCount );

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

