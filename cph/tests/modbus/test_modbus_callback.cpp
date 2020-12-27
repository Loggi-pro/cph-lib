#include <unity/unity_fixture.h>
#include "cph/modbus/mb.h"
#include "cph/modbus/mb_registers.h"
#define REG_INPUT_NREGS			  16
#define REG_HOLDING_NREGS		  120
#define REG_COILS_NREGS			  176
#define REG_DISCRETE_NREGS		  250



typedef ModbusRegs<0, REG_INPUT_NREGS, REG_HOLDING_NREGS, REG_COILS_NREGS, REG_DISCRETE_NREGS>
ModbusRegisters;
static Modbus mb;
static ModbusSpy mbSpy = ModbusSpy(mb);


static void checkCoilsBuf(bool* expectedCoilsBuf, u08 BufSize) {
	if (BufSize > REG_COILS_NREGS) {
		TEST_FAIL_MESSAGE("Expectation array size exceeds actual buffer size");
	}

	for (int i = 0; i < BufSize; i++) {
		TEST_ASSERT_EQUAL_INT(expectedCoilsBuf[i], ModbusRegisters::usRegCoilsBuf[i]);
	}
}


TEST_GROUP(ModbusCallbacksTestGroup);

TEST_SETUP(ModbusCallbacksTestGroup) {
	// Clear coils registers
	for (int i = 0; i < REG_COILS_NREGS; i++) {
		ModbusRegisters::usRegCoilsBuf[i] = false;
	}

	mb.setRegisterInputCallback(&ModbusRegisters::readInputRegister);
	mb.setRegisterHoldingCallback(&ModbusRegisters::readHoldingRegister,
	                              &ModbusRegisters::writeHoldingRegister);
	mb.setRegisterCoilsCallback(&ModbusRegisters::readCoil, ModbusRegisters::writeCoil);
	mb.setRegisterDiscreteCallback(&ModbusRegisters::readDiscrete);
}

TEST_TEAR_DOWN(ModbusCallbacksTestGroup) {
}

TEST(ModbusCallbacksTestGroup, WriteSingleCoil) { // Write single coil
	const u08 expectationBufSize = 4;
	u08 pucRegBuffer[1] = { 0x01 };
	u16 usAddress = 0 + ADDRESS_OFFSET;
	u16 usNCoils = 1;
	eMBRegisterMode eMode = MB_REG_WRITE;
	MBErrorCode errorCode;
	bool expectedCoilsBuf[expectationBufSize] = { true, false, false, false };
	errorCode = mbSpy.callRegisterCoilsHandler(pucRegBuffer, usAddress, usNCoils, eMode);

	if (errorCode != MB_ENOERR) {
		TEST_FAIL_MESSAGE("Function returned error code");
	}

	checkCoilsBuf(expectedCoilsBuf, expectationBufSize);
}

TEST(ModbusCallbacksTestGroup, WriteFewCoils) { // Write few coils (but < 8)
	const u08 expectationBufSize = 4;
	u08 pucRegBuffer[1] = { 0x0A };  // 00001010 0x0A
	u16 usAddress = 0 + ADDRESS_OFFSET;
	u16 usNCoils = 4;
	eMBRegisterMode eMode = MB_REG_WRITE;
	MBErrorCode errorCode;
	bool expectedCoilsBuf[expectationBufSize] = { false, true, false, true };
	errorCode = mbSpy.callRegisterCoilsHandler(pucRegBuffer, usAddress, usNCoils, eMode);

	if (errorCode != MB_ENOERR) {
		TEST_FAIL_MESSAGE("Function returned error code");
	}

	checkCoilsBuf(expectedCoilsBuf, expectationBufSize);
}

TEST(ModbusCallbacksTestGroup, WriteWrongCoilAddress) { // Attempt to write wrong address
	const u08 expectationBufSize = 4;
	u08 pucRegBuffer[1] = { 0x0A };
	u16 usAddress = REG_COILS_NREGS + ADDRESS_OFFSET;  // Wrong address
	u16 usNCoils = 4;
	eMBRegisterMode eMode = MB_REG_WRITE;
	MBErrorCode errorCode;
	bool expectedCoilsBuf[expectationBufSize] = { false, true, false, true };
	errorCode = mbSpy.callRegisterCoilsHandler(pucRegBuffer, usAddress, usNCoils, eMode);

	if (errorCode != MB_ENOREG) {
		TEST_FAIL_MESSAGE("errorCode must be MB_ENOREG");
	}

	// zero usNDiscrete
	usAddress = 0 + ADDRESS_OFFSET;
	usNCoils = 0;
	errorCode = mbSpy.callRegisterCoilsHandler(pucRegBuffer, usAddress, usNCoils, eMode);

	if (errorCode != MB_ENOREG) {
		TEST_FAIL_MESSAGE("Function must return error code MB_ENOREG");
	}

	// To many registers to write
	usNCoils = 11;
	usAddress = REG_COILS_NREGS - 10 + ADDRESS_OFFSET;
	errorCode = mbSpy.callRegisterCoilsHandler(pucRegBuffer, usAddress, usNCoils, eMode);

	if (errorCode != MB_ENOREG) {
		TEST_FAIL_MESSAGE("Function must return error code MB_ENOREG");
	}
}

TEST(ModbusCallbacksTestGroup, WriteTooManyCoils) { // Attempt to write too many registers
	u08 pucRegBuffer[1] = { 0x0A };
	u16 usAddress = 1 + ADDRESS_OFFSET;
	u16 usNCoils = REG_COILS_NREGS;
	eMBRegisterMode eMode = MB_REG_WRITE;
	MBErrorCode errorCode;
	errorCode = mbSpy.callRegisterCoilsHandler(pucRegBuffer, usAddress, usNCoils, eMode);

	if (errorCode != MB_ENOREG) {
		TEST_FAIL_MESSAGE("errorCode must be MB_ENOREG");
	}
}

TEST(ModbusCallbacksTestGroup, WriteMultipleCoils) { // Write multiple coil ( > 8) from zero address
	const u08 expectationBufSize = 24;
	u08 pucRegBuffer[3] = { 0x80, 0x01, 0xFF };
	u16 usAddress = 0 + ADDRESS_OFFSET;
	u16 usNCoils = 24;
	eMBRegisterMode eMode = MB_REG_WRITE;
	MBErrorCode errorCode;
	bool expectedCoilsBuf[expectationBufSize] = { false, false, false, false,  false, false, false, true,
	                                              true,  false, false, false,  false, false, false, false,
	                                              true,  true,  true,  true,   true,  true,  true,  true
	                                            };
	errorCode = mbSpy.callRegisterCoilsHandler(pucRegBuffer, usAddress, usNCoils, eMode);

	if (errorCode != MB_ENOERR) {
		TEST_FAIL_MESSAGE("Function returned error code");
	}

	checkCoilsBuf(expectedCoilsBuf, expectationBufSize);
}

TEST(ModbusCallbacksTestGroup, Write8CoilsToNonZeroAddress) { // Write 8 coils to nonzero address
	const u08 expectationBufSize = 24;
	u08 pucRegBuffer[1] = { 0x83 }; // 0x83 10000011
	u16 usAddress = 7 + ADDRESS_OFFSET;
	u16 usNCoils = 8;
	eMBRegisterMode eMode = MB_REG_WRITE;
	MBErrorCode errorCode;
	bool expectedCoilsBuf[expectationBufSize] = { false, false, false, false,	false, false, false, true,
	                                              true,  false, false, false,	false, false, true,  false,
	                                              false, false, false, false,	false, false, false, false
	                                            };
	errorCode = mbSpy.callRegisterCoilsHandler(pucRegBuffer, usAddress, usNCoils, eMode);

	if (errorCode != MB_ENOERR) {
		TEST_FAIL_MESSAGE("Function returned error code");
	}

	checkCoilsBuf(expectedCoilsBuf, expectationBufSize);
}

TEST(ModbusCallbacksTestGroup,
     ReadSingleCoilOnZeroAddress) { // Read single register on zero address
	u08 pucRegBuffer[10];
	u16 usAddress = 0 + ADDRESS_OFFSET;
	u16 usNCoils = 1;
	eMBRegisterMode eMode = MB_REG_READ;
	MBErrorCode errorCode;
	// Clear buffer
	pucRegBuffer[0] = 0x00;
	// Set register value
	ModbusRegisters::usRegCoilsBuf[0] = true;
	errorCode = mbSpy.callRegisterCoilsHandler(pucRegBuffer, usAddress, usNCoils, eMode);

	if (errorCode != MB_ENOERR) {
		TEST_FAIL_MESSAGE("Function returned error code");
	}

	TEST_ASSERT_EQUAL_INT(0x01, pucRegBuffer[0]);
}

TEST(ModbusCallbacksTestGroup,
     ReadSingleCoilOnNonZeroAddress) { // Read single register on nonzero address
	u08 pucRegBuffer[10];
	u16 usAddress = 1 + ADDRESS_OFFSET;
	u16 usNCoils = 1;
	eMBRegisterMode eMode = MB_REG_READ;
	MBErrorCode errorCode;
	// Clear buffer
	pucRegBuffer[0] = 0x00;
	// Set register value
	ModbusRegisters::usRegCoilsBuf[0] = false;
	ModbusRegisters::usRegCoilsBuf[1] = true;
	ModbusRegisters::usRegCoilsBuf[2] = false;
	ModbusRegisters::usRegCoilsBuf[3] = false;
	errorCode = mbSpy.callRegisterCoilsHandler(pucRegBuffer, usAddress, usNCoils, eMode);

	if (errorCode != MB_ENOERR) {
		TEST_FAIL_MESSAGE("Function returned error code");
	}

	TEST_ASSERT_EQUAL_INT(0x01, pucRegBuffer[0]);
}

TEST(ModbusCallbacksTestGroup, ReadMultipleCoils) { // Read multiple registers
	u08 pucRegBuffer[10];
	u16 usAddress = 0 + ADDRESS_OFFSET;
	u16 usNCoils = 16;
	eMBRegisterMode eMode = MB_REG_READ;
	MBErrorCode errorCode;
	// Clear buffer
	pucRegBuffer[0] = 0x00;
	pucRegBuffer[1] = 0x00;
	pucRegBuffer[2] = 0x00;
	// Set register value
	ModbusRegisters::usRegCoilsBuf[0] = false;
	ModbusRegisters::usRegCoilsBuf[1] = false;
	ModbusRegisters::usRegCoilsBuf[2] = false;
	ModbusRegisters::usRegCoilsBuf[3] = false;
	ModbusRegisters::usRegCoilsBuf[4] = true;
	ModbusRegisters::usRegCoilsBuf[5] = true;
	ModbusRegisters::usRegCoilsBuf[6] = true;
	ModbusRegisters::usRegCoilsBuf[7] = true;
	ModbusRegisters::usRegCoilsBuf[8] = true;
	ModbusRegisters::usRegCoilsBuf[9] = true;
	ModbusRegisters::usRegCoilsBuf[10] = true;
	ModbusRegisters::usRegCoilsBuf[11] = true;
	ModbusRegisters::usRegCoilsBuf[12] = false;
	ModbusRegisters::usRegCoilsBuf[13] = true;
	ModbusRegisters::usRegCoilsBuf[14] = true;
	ModbusRegisters::usRegCoilsBuf[15] = false;
	errorCode = mbSpy.callRegisterCoilsHandler(pucRegBuffer, usAddress, usNCoils, eMode);

	if (errorCode != MB_ENOERR) {
		TEST_FAIL_MESSAGE("Function returned error code");
	}

	TEST_ASSERT_EQUAL_INT(0xF0, pucRegBuffer[0]);
	TEST_ASSERT_EQUAL_INT(0x6F, pucRegBuffer[1]);
	TEST_ASSERT_EQUAL_INT(0x00, pucRegBuffer[2]);
}

TEST(ModbusCallbacksTestGroup,
     ReadNumOfCoilsNotMultiple8) { // If number of read bits non multiple 8
	// then last bits of the packed buffer fills with zeros
	u08 pucRegBuffer[10];
	u16 usAddress = 0 + ADDRESS_OFFSET;
	u16 usNCoils = 12;
	eMBRegisterMode eMode = MB_REG_READ;
	MBErrorCode errorCode;
	// Clear buffer
	pucRegBuffer[0] = 0xFF;
	pucRegBuffer[1] = 0xFF;
	pucRegBuffer[2] = 0xFF;
	ModbusRegisters::usRegCoilsBuf[0] = true;
	ModbusRegisters::usRegCoilsBuf[1] = false;
	ModbusRegisters::usRegCoilsBuf[2] = true;
	ModbusRegisters::usRegCoilsBuf[3] = false;
	ModbusRegisters::usRegCoilsBuf[4] = true;
	ModbusRegisters::usRegCoilsBuf[5] = true;
	ModbusRegisters::usRegCoilsBuf[6] = true;
	ModbusRegisters::usRegCoilsBuf[7] = true;
	ModbusRegisters::usRegCoilsBuf[8] = true;
	ModbusRegisters::usRegCoilsBuf[9] = false;
	ModbusRegisters::usRegCoilsBuf[10] = true;
	ModbusRegisters::usRegCoilsBuf[11] = true;
	ModbusRegisters::usRegCoilsBuf[12] = false;
	ModbusRegisters::usRegCoilsBuf[13] = true;
	ModbusRegisters::usRegCoilsBuf[14] = true;
	ModbusRegisters::usRegCoilsBuf[15] = false;
	errorCode = mbSpy.callRegisterCoilsHandler(pucRegBuffer, usAddress, usNCoils, eMode);

	if (errorCode != MB_ENOERR) {
		TEST_FAIL_MESSAGE("Function returned error code");
	}

	TEST_ASSERT_EQUAL_INT(0xF5, pucRegBuffer[0]); // F5  11110101
	TEST_ASSERT_EQUAL_INT(0x0D, pucRegBuffer[1]); // 0D 00001101
}

TEST(ModbusCallbacksTestGroup, HoldingCBOutOfAddress) { // Not clear pucRegBuffer works well
	u08 pucRegBuffer[10];
	u16 usAddress = REG_HOLDING_NREGS + ADDRESS_OFFSET;
	u16 usNCoils = 1;
	eMBRegisterMode eMode = MB_REG_READ;
	MBErrorCode errorCode;
	errorCode = mbSpy.callRegisterHoldingHandler(pucRegBuffer, usAddress, usNCoils, eMode);

	if (errorCode != MB_ENOREG) {
		TEST_FAIL_MESSAGE("Function must return error code MB_ENOREG");
	}

	eMode = MB_REG_WRITE;
	errorCode = mbSpy.callRegisterHoldingHandler(pucRegBuffer, usAddress, usNCoils, eMode);

	if (errorCode != MB_ENOREG) {
		TEST_FAIL_MESSAGE("Function must return error code MB_ENOREG");
	}

	// Zero num of register to read/write
	usAddress = 0 + ADDRESS_OFFSET;
	usNCoils = 0;
	eMode = MB_REG_READ;
	errorCode = mbSpy.callRegisterHoldingHandler(pucRegBuffer, usAddress, usNCoils, eMode);

	if (errorCode != MB_ENOREG) {
		TEST_FAIL_MESSAGE("Function must return error code MB_ENOREG");
	}

	usAddress = 0 + ADDRESS_OFFSET;
	usNCoils = 0;
	eMode = MB_REG_WRITE;
	errorCode = mbSpy.callRegisterHoldingHandler(pucRegBuffer, usAddress, usNCoils, eMode);

	if (errorCode != MB_ENOREG) {
		TEST_FAIL_MESSAGE("Function must return error code MB_ENOREG");
	}

	// To many registers to read/write
	usAddress = REG_HOLDING_NREGS + ADDRESS_OFFSET - 10;
	usNCoils = 11;
	eMode = MB_REG_WRITE;
	errorCode = mbSpy.callRegisterHoldingHandler(pucRegBuffer, usAddress, usNCoils, eMode);

	if (errorCode != MB_ENOREG) {
		TEST_FAIL_MESSAGE("Function must return error code MB_ENOREG");
	}

	usAddress = REG_HOLDING_NREGS + ADDRESS_OFFSET - 10;
	usNCoils = 11;
	eMode = MB_REG_READ;
	errorCode = mbSpy.callRegisterHoldingHandler(pucRegBuffer, usAddress, usNCoils, eMode);

	if (errorCode != MB_ENOREG) {
		TEST_FAIL_MESSAGE("Function must return error code MB_ENOREG");
	}
}

TEST(ModbusCallbacksTestGroup, InputRegistersCBOutOfAddress) { // Not clear pucRegBuffer works well
	u08 pucRegBuffer[20];
	u16 usAddress = REG_INPUT_NREGS + ADDRESS_OFFSET;
	u16 usNRegs = 1;
	MBErrorCode errorCode;
	// usAddress is Out of memory
	errorCode = mbSpy.callRegisterInputHandler(pucRegBuffer, usAddress, usNRegs);

	if (errorCode != MB_ENOREG) {
		TEST_FAIL_MESSAGE("Function must return error code MB_ENOREG");
	}

	// zero usNRegs
	usAddress = 0 + ADDRESS_OFFSET;
	usNRegs = 0;
	errorCode = mbSpy.callRegisterInputHandler(pucRegBuffer, usAddress, usNRegs);

	if (errorCode != MB_ENOREG) {
		TEST_FAIL_MESSAGE("Function must return error code MB_ENOREG");
	}

	// To many registers to read
	if (REG_INPUT_NREGS < 10) {
		TEST_FAIL_MESSAGE("REG_INPUT_NREGS must be >= 10 in this test");
	}

	usNRegs = 11;
	usAddress = REG_INPUT_NREGS - 10 + ADDRESS_OFFSET;

	for (uint16_t i = 0; i < REG_INPUT_NREGS; ++i) {
		ModbusRegisters::usRegInputBuf[i] = i + 1;
	}

	errorCode = mbSpy.callRegisterInputHandler(pucRegBuffer, usAddress, usNRegs);

	if (errorCode != MB_ENOREG) {
		TEST_FAIL_MESSAGE("Function must return error code MB_ENOREG");
	}
}

TEST(ModbusCallbacksTestGroup,
     DiscretRegistersCBOutOfAddress) { // Not clear pucRegBuffer works well
	u08 pucRegBuffer[10];
	u16 usAddress = REG_DISCRETE_NREGS + ADDRESS_OFFSET;
	u16 usNDiscrete = 1;
	MBErrorCode errorCode;
	// usAddress is Out of memory
	errorCode = mbSpy.callRegisterDiscreteHandler(pucRegBuffer, usAddress, usNDiscrete);

	if (errorCode != MB_ENOREG) {
		TEST_FAIL_MESSAGE("Function must return error code MB_ENOREG");
	}

	// zero usNDiscrete
	usAddress = 0 + ADDRESS_OFFSET;
	usNDiscrete = 0;
	errorCode = mbSpy.callRegisterDiscreteHandler(pucRegBuffer, usAddress, usNDiscrete);

	if (errorCode != MB_ENOREG) {
		TEST_FAIL_MESSAGE("Function must return error code MB_ENOREG");
	}

	// To many registers to read
	usNDiscrete = 11;
	usAddress = REG_DISCRETE_NREGS - 10 + ADDRESS_OFFSET;
	errorCode = mbSpy.callRegisterDiscreteHandler(pucRegBuffer, usAddress, usNDiscrete);

	if (errorCode != MB_ENOREG) {
		TEST_FAIL_MESSAGE("Function must return error code MB_ENOREG");
	}
}

TEST_GROUP_RUNNER(ModbusCallbacksTestGroup) {
	RUN_TEST_CASE(ModbusCallbacksTestGroup, WriteSingleCoil);
	RUN_TEST_CASE(ModbusCallbacksTestGroup, WriteFewCoils);
	RUN_TEST_CASE(ModbusCallbacksTestGroup, WriteWrongCoilAddress);
	RUN_TEST_CASE(ModbusCallbacksTestGroup, WriteTooManyCoils);
	RUN_TEST_CASE(ModbusCallbacksTestGroup, WriteMultipleCoils);
	RUN_TEST_CASE(ModbusCallbacksTestGroup, Write8CoilsToNonZeroAddress);
	RUN_TEST_CASE(ModbusCallbacksTestGroup, ReadSingleCoilOnZeroAddress);
	RUN_TEST_CASE(ModbusCallbacksTestGroup, ReadSingleCoilOnNonZeroAddress);
	RUN_TEST_CASE(ModbusCallbacksTestGroup, ReadMultipleCoils);
	RUN_TEST_CASE(ModbusCallbacksTestGroup, ReadNumOfCoilsNotMultiple8);
	RUN_TEST_CASE(ModbusCallbacksTestGroup, HoldingCBOutOfAddress)
	RUN_TEST_CASE(ModbusCallbacksTestGroup, InputRegistersCBOutOfAddress);
	RUN_TEST_CASE(ModbusCallbacksTestGroup, DiscretRegistersCBOutOfAddress);
}
