
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbframe.h"
#include "mbconfig.h"

/* ----------------------- Start implementation -----------------------------*/


MBException mbFuncReportSlaveID(Modbus& obj, uint8_t* frame, uint16_t* length ) {
	#if MB_FUNC_OTHER_REP_SLAVEID_ENABLED > 0
	frame[MB_PDU_DATA_OFF] = obj._mbAddress; //self ID
	frame[MB_PDU_DATA_OFF + 1] = obj._mbState == Modbus::STATE_ENABLED ? 0xFF : 0; //running flag
	*length = (uint16_t)( MB_PDU_DATA_OFF + 2 );
	return MB_EX_NONE;
	#else
	return MB_EX_ILLEGAL_FUNCTION;
	#endif
}

