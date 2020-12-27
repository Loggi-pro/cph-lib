
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbframe.h"
#include "mbconfig.h"
#include "mbutils.h"
/* ----------------------- Defines ------------------------------------------*/
#define MB_PDU_FUNC_READ_ADDR_OFF           ( MB_PDU_DATA_OFF )
#define MB_PDU_FUNC_READ_REGCNT_OFF         ( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_FUNC_READ_SIZE               ( 4 )
#define MB_PDU_FUNC_READ_REGCNT_MAX         ( 0x007D )

#define MB_PDU_FUNC_READ_RSP_BYTECNT_OFF    ( MB_PDU_DATA_OFF )

/* ----------------------- Static functions ---------------------------------*/
MBException    prveMBError2Exception( MBErrorCode errorCode );

/* ----------------------- Start implementation -----------------------------*/

MBException mbFuncReadInputRegister(Modbus& obj, uint8_t* frame, uint16_t* length ) {
	#if MB_FUNC_READ_INPUT_ENABLED > 0
	MBException status = MB_EX_NONE;

	if (*length != (MB_PDU_FUNC_READ_SIZE + MB_PDU_SIZE_MIN)) {
		/* Can't be a valid read input register request because the length
		* is incorrect. */
		return MB_EX_ILLEGAL_DATA_VALUE;
	}

	uint16_t regAddress = readAsU16_HighEndian(&frame[MB_PDU_FUNC_READ_ADDR_OFF]) + 1;
	uint16_t regCount = readAsU16_HighEndian(&frame[MB_PDU_FUNC_READ_REGCNT_OFF]);

	/* Check if the number of registers to read is valid. If not
	 * return Modbus illegal data value exception.
	 */
	if ((regCount >= 1) && (regCount < MB_PDU_FUNC_READ_REGCNT_MAX)) {
		/* Set the current PDU data pointer to the beginning. */
		uint8_t*  frameCur = &frame[MB_PDU_FUNC_OFF];
		*length = MB_PDU_FUNC_OFF;
		/* First byte contains the function code. */
		*frameCur++ = MB_FUNC_READ_INPUT_REGISTER;
		*length += 1;
		/* Second byte in the response contain the number of bytes. */
		*frameCur++ = ( uint8_t )( regCount * 2 );
		*length += 1;
		MBErrorCode regStatus = obj.handlerRegInput(frameCur, regAddress, regCount);

		/* If an error occured convert it into a Modbus exception. */
		if (regStatus != MB_ENOERR) {
			status = prveMBError2Exception(regStatus);
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

