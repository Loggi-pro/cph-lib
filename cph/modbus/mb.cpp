
/* ----------------------- System includes ----------------------------------*/
#include "stdlib.h"
#include "string.h"


/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbconfig.h"
#include "mbframe.h"






/* Functions pointer which are initialized in eMBInit( ). Depending on the
 * mode (RTU or ASCII) the are set to the correct implementations.
 */

static MBErrorCode _ReadRegCallbackStub(uint16_t* out, uint16_t address, uint16_t count) {
	return MBErrorCode::MB_ENOREG;
};
static MBErrorCode _WriteRegCallbackStub(const uint16_t* data, uint16_t address, uint16_t count) {
	return MBErrorCode::MB_ENOREG;
};
static MBErrorCode _BitReadCallbackStub(bit_array_t& bits, uint16_t address, uint16_t count) {
	return MBErrorCode::MB_ENOREG;
};
static MBErrorCode _BitWriteCallbackStub(const bit_array_t& bits, uint16_t address,
        uint16_t count) {
	return MBErrorCode::MB_ENOREG;
};


Modbus::Modbus():
	_mbAddress(0),
	_modbusObject(),
	_mbState(STATE_NOT_INITIALIZED),
	_cbRegInputRead(_ReadRegCallbackStub),
	_cbRegHoldingRead(_ReadRegCallbackStub),
	_cbRegHoldingWrite(_WriteRegCallbackStub),
	_cbRegCoilsRead(_BitReadCallbackStub),
	_cbRegCoilsWrite(_BitWriteCallbackStub),
	_cbRegDiscreteRead(_BitReadCallbackStub) {
}



MBException	_StubCommandHandler(Modbus& obj, uint8_t* frame, uint16_t* length) {
	return MB_EX_ILLEGAL_FUNCTION;
}
extern MBException	mbFuncReportSlaveID(Modbus& obj, uint8_t* frame, uint16_t* length);
extern MBException    mbFuncReadInputRegister(Modbus& obj, uint8_t* frame, uint16_t* length);
extern MBException    mbFuncReadHoldingRegister(Modbus& obj, uint8_t* frame, uint16_t* length);
extern MBException    mbFuncWriteHoldingRegister(Modbus& obj, uint8_t* frame, uint16_t* length);
extern MBException    mbFuncWriteMultipleHoldingRegister(Modbus& obj, uint8_t* frame,
        uint16_t* length);
extern MBException    mbFuncReadCoils(Modbus& obj, uint8_t* frame, uint16_t* length);
extern MBException    mbFuncWriteCoil(Modbus& obj, uint8_t* frame, uint16_t* length);
extern MBException    mbFuncWriteMultipleCoils(Modbus& obj, uint8_t* frame, uint16_t* length);
extern MBException    mbFuncReadDiscreteInputs(Modbus& obj, uint8_t* frame, uint16_t* length);
extern MBException    mbFuncReadWriteMultipleHoldingRegister(Modbus& obj, uint8_t* pucFrame,
        uint16_t* length);






/* An array of Modbus functions handlers which associates Modbus function
 * codes with implementing functions.
 *
 */
enum HandlerCode {
	REPORT_SLAVEID = MB_FUNC_OTHER_REPORT_SLAVEID,
	READ_INPUT_REGISTER = MB_FUNC_READ_INPUT_REGISTER,
	READ_HOLDING_REGISTER = MB_FUNC_READ_HOLDING_REGISTER,
	WRITE_MULTIPLE_REGISTERS = MB_FUNC_WRITE_MULTIPLE_REGISTERS,
	WRITE_REGISTER = MB_FUNC_WRITE_REGISTER,
	READWRITE_MULTIPLE_REGISTERS = MB_FUNC_READWRITE_MULTIPLE_REGISTERS,
	READ_COILS = MB_FUNC_READ_COILS,
	WRITE_SINGLE_COIL = MB_FUNC_WRITE_SINGLE_COIL,
	WRITE_MULTIPLE_COILS = MB_FUNC_WRITE_MULTIPLE_COILS,
	READ_DISCRETE_INPUTS = MB_FUNC_READ_DISCRETE_INPUTS
};

static MBFunctionHandler getHandler(HandlerCode code) {
	switch (code) {
	case REPORT_SLAVEID: return mbFuncReportSlaveID;

	case READ_INPUT_REGISTER: return mbFuncReadInputRegister;

	case READ_HOLDING_REGISTER: return mbFuncReadHoldingRegister;

	case WRITE_MULTIPLE_REGISTERS: return mbFuncWriteMultipleHoldingRegister;

	case WRITE_REGISTER: return mbFuncWriteHoldingRegister;

	case READWRITE_MULTIPLE_REGISTERS: return mbFuncReadWriteMultipleHoldingRegister;

	case READ_COILS: return mbFuncReadCoils;

	case WRITE_SINGLE_COIL: return mbFuncWriteCoil;

	case WRITE_MULTIPLE_COILS: return mbFuncWriteMultipleCoils;

	case READ_DISCRETE_INPUTS: return mbFuncReadDiscreteInputs;

	default:
		return _StubCommandHandler;
	}
}

/* ----------------------- Start implementation -----------------------------*/


static bool isValidAddress(uint8_t address) {
	if ((address == MB_ADDRESS_BROADCAST) ||
	        (address < MB_ADDRESS_MIN) || (address > MB_ADDRESS_MAX)) {
		return false;
	}

	return true;
}

ModbusRTU& Modbus::instance() {
	return _modbusObject;
}

MBErrorCode Modbus::init(TransportAdapter* transport, TimerAdapter* timer, uint8_t slaveAddress) {
	/* check preconditions */
	if (!isValidAddress(slaveAddress)) { return MB_EINVAL; }

	_mbAddress = slaveAddress;
	MBErrorCode status = _modbusObject.init(transport, timer, &_queue);

	if (status != MB_ENOERR) { return status; }

	_queue.init();
	_mbState = STATE_DISABLED;
	return status;
}



MBErrorCode Modbus::close() {
	MBErrorCode    status = MB_ENOERR;

	if ( _mbState == STATE_DISABLED ) {
		_modbusObject.close();
	} else {
		status = MB_EILLSTATE;
	}

	return status;
}

MBErrorCode Modbus::enable() {
	MBErrorCode    status = MB_ENOERR;

	if ( _mbState == STATE_DISABLED ) {
		/* Activate the protocol stack. */
		_modbusObject.start();
		_mbState = STATE_ENABLED;
	} else {
		status = MB_EILLSTATE;
	}

	return status;
}

MBErrorCode Modbus::disable() {
	MBErrorCode    status;

	if ( _mbState == STATE_ENABLED ) {
		_modbusObject.stop();
		_mbState = STATE_DISABLED;
		status = MB_ENOERR;
	} else if ( _mbState == STATE_DISABLED ) {
		status = MB_ENOERR;
	} else {
		status = MB_EILLSTATE;
	}

	return status;
}
MBErrorCode Modbus::poll() {
	static uint8_t*   frame;
	static uint8_t    rxAddress;
	static uint8_t    functionCode;
	static uint16_t   length;
	static MBException exception;
	MBErrorCode    status = MB_ENOERR;
	ModbusEventQueue::MBEventType event;

	/* Check if the protocol stack is ready. */
	if ( _mbState != STATE_ENABLED ) {return MB_EILLSTATE;}

	/* Check if there is a event available. If not return control to caller.
	 * Otherwise we will handle the event. */
	if (!_queue.getEvent(&event)) { return MB_ENOERR;}

	switch (event) {
	case ModbusEventQueue::EV_READY: break;

	case ModbusEventQueue::EV_FRAME_RECEIVED:
		status = _modbusObject.receive( &rxAddress, &frame, &length );

		//cout << "Frame recived\r\n";

		if (status != MB_ENOERR) { break; }

		/* Check if the frame is for us. If not ignore the frame. */
		if ((rxAddress == _mbAddress) || (rxAddress == MB_ADDRESS_BROADCAST)) {
			_queue.postEvent(ModbusEventQueue::EV_EXECUTE);
		}

		break;

	case ModbusEventQueue::EV_EXECUTE:
		functionCode = frame[MB_PDU_FUNC_OFF];
		exception = getHandler((HandlerCode)functionCode)(*this, frame, &length);

		/* If the request was not sent to the broadcast address we
		 * return a reply. */

		if (rxAddress != MB_ADDRESS_BROADCAST) {
			if (exception != MB_EX_NONE) {
				/* An exception occurred. Build an error frame. */
				length = 0;
				frame[length++] = (uint8_t)(functionCode | MB_FUNC_ERROR);
				frame[length++] = exception;
			}

			status = _modbusObject.send(_mbAddress, frame, length);
		}

		break;

	case ModbusEventQueue::EV_FRAME_SENT: break;
	}

	return MB_ENOERR;
}

void Modbus::setRegisterInputCallback(mbReadRegisterCallback_t f) {
	_cbRegInputRead = f == nullptr ? _ReadRegCallbackStub : f;
}

void Modbus::setRegisterHoldingCallback(mbReadRegisterCallback_t read,
                                        mbWriteRegisterCallback_t write) {
	_cbRegHoldingRead = read == nullptr ? _ReadRegCallbackStub : read;
	_cbRegHoldingWrite = write == nullptr ? _WriteRegCallbackStub : write;
}
void Modbus::setRegisterCoilsCallback(mbReadBitCallback_t read, mbWriteBitCallback_t write) {
	_cbRegCoilsRead = read == nullptr ? _BitReadCallbackStub : read;
	_cbRegCoilsWrite = write == nullptr ? _BitWriteCallbackStub : write;
}
void Modbus::setRegisterDiscreteCallback(mbReadBitCallback_t f) {
	_cbRegDiscreteRead = f == nullptr ? _BitReadCallbackStub : f;
}


MBErrorCode	Modbus::handlerRegInput(uint8_t* regBuffer, uint16_t address, uint16_t nRegs) {
	if (nRegs == 0) { return MB_ENOREG; } //if usNRegs==0 return error

	address = address - ADDRESS_OFFSET;  // Compensate Modbus lib address incrementation
	uint16_t* buffer = (uint16_t*)regBuffer;
	MBErrorCode status = _cbRegInputRead(buffer, address, nRegs);
	return status;
}


MBErrorCode   Modbus::handlerRegHolding(uint8_t* regBuffer, uint16_t address, uint16_t nRegs,
                                        eMBRegisterMode mode) {
	if (nRegs == 0) { return MB_ENOREG; } //if usNRegs==0 return error

	MBErrorCode status = MB_ENOREG;
	address = address - ADDRESS_OFFSET;// Compensate Modbus lib address incrementation
	uint16_t* buffer = (uint16_t*)regBuffer;

	if (mode == MB_REG_READ) {
		status = _cbRegHoldingRead(buffer, address, nRegs);
	} else {
		status = _cbRegHoldingWrite(buffer, address, nRegs);
	}

	return status;
}




MBErrorCode    Modbus::handlerRegCoils(uint8_t* regBuffer, uint16_t address,
                                       uint16_t usNCoils, eMBRegisterMode mode) {
	if (usNCoils == 0) { return MB_ENOREG; } //if usNRegs==0 return error

	MBErrorCode status = MB_ENOREG;
	address = address - ADDRESS_OFFSET; // Compensate Modbus lib address incrementation
	uint8_t* buffer = regBuffer;
	bit_array_t bits = { buffer };

	if (mode == MB_REG_READ) {
		bits.clear_bits(usNCoils);
		status = _cbRegCoilsRead(bits, address, usNCoils);
	} else {
		status = _cbRegCoilsWrite(bits, address, usNCoils);
	}

	return status;
}



MBErrorCode    Modbus::handlerRegDiscrete(uint8_t* regBuffer, uint16_t address,
        uint16_t nDiscrete) {
	if (nDiscrete == 0) { return MB_ENOREG; } //if usNDiscrete==0 return error

	MBErrorCode status = MB_ENOREG;
	address = address - ADDRESS_OFFSET; // Compensate Modbus lib address incrementation
	uint8_t* buffer = (uint8_t*)regBuffer;
	//clearBits(buffer, nDiscrete);
	bit_array_t bits = {buffer};
	bits.clear_bits(nDiscrete);
	status = _cbRegDiscreteRead(bits, address, nDiscrete);
	return status;
}