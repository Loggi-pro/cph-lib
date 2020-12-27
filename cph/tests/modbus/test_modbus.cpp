#include <unity/unity_fixture.h>
#include "cph/modbus/mb.h"
#include "cph/modbus/mbport.h"
#include "cph/modbus/mbutils.h"

#include "cph/modbus/mb_registers.h"
#define REG_INPUT_NREGS			  16
#define REG_HOLDING_NREGS		  120
#define REG_COILS_NREGS			  176
#define REG_DISCRETE_NREGS		  250

//create standart modbus registers
typedef ModbusRegs<0, REG_INPUT_NREGS, REG_HOLDING_NREGS, REG_COILS_NREGS, REG_DISCRETE_NREGS>ModbusRegisters;




// Protocol log examples are taken from https://ipc2u.ru/articles/prostye-resheniya/modbus-rtu/
// CRC calculator http://crccalc.com/


static Modbus mb;
static ModbusSpy mbSpy = ModbusSpy(mb);


struct FakeTransportAdapter : TransportAdapter {

	enum { LOG_DATA_LENGTH = 30 };
	unsigned char logData[LOG_DATA_LENGTH];
	int logEndIndex = 0;
	int logStartIndex = 0;
	unsigned char UDR;
	void(*callbackRx)(const uint8_t&) = nullptr;
	bool(*callbackTx)(uint8_t* data) = nullptr;


	virtual bool init() override {
		callbackRx = [](const uint8_t& byte) {
			mb.instance().handleReceived(byte);
		};
		return true;
	}
	virtual  bool writeByte(u08 byte) override {
		logData[logEndIndex] = byte;
		logEndIndex++;
		return true;
	}
	virtual void setDirection(MBDirection dir) override {
	}
	virtual uint32_t getBaudrate() override {
		return 19200;
	}

	void clearLog() {
		for (int i = 0; i < LOG_DATA_LENGTH; i++) {
			logData[i] = 0;
		}

		logEndIndex = 0;
		logStartIndex = 0;
	}
	uint8_t tranceiveCount()const {
		return logEndIndex - logStartIndex;
	}
	bool outLogPop(uint8_t* data) {
		if (!tranceiveCount()) {return false;}

		*data = logData[logStartIndex++];
		return true;
	}
	void spyReceiveByte(uint8_t byte) {
		callbackRx(byte);
	}
};







struct FakeTimerAdapterImpl : TimerAdapter {
	static void TimerExpired() {
	}
	virtual bool	init(uint32_t usTickTime) override {
		return true;
	}
	virtual void close(void) override {
	}
	virtual void enable(void) override {
	}
	virtual void reset() override {
	}
	virtual void disable() override {
	}
	virtual void delay(u16 usTimeOutMS)override {
		return;
	}
};


static FakeTimerAdapterImpl timerAdapter;

static FakeTransportAdapter transport;



#define checkSendedBytes(sendedByteCount, sendedBytes){\
		u08 byte = 0;\
		u08 counter = 0;\
		TEST_ASSERT_EQUAL(sendedByteCount,transport.tranceiveCount());\
		while (transport.outLogPop(&byte)) {\
			TEST_ASSERT_EQUAL_INT(sendedBytes[counter++], byte);\
		}\
	}








TEST_GROUP(modbus);

TEST_SETUP(modbus) {
	transport.clearLog();
	mb.setRegisterInputCallback(&ModbusRegisters::readInputRegister);
	mb.setRegisterHoldingCallback(&ModbusRegisters::readHoldingRegister,
	                              &ModbusRegisters::writeHoldingRegister);
	mb.setRegisterCoilsCallback(&ModbusRegisters::readCoil, ModbusRegisters::writeCoil);
	mb.setRegisterDiscreteCallback(&ModbusRegisters::readDiscrete);
}

TEST_TEAR_DOWN(modbus) {
}
TEST(modbus, TestCRC) {
	u08 var = 1;
	u16 res = calc_crc16(&var, 1);
	TEST_ASSERT_EQUAL_HEX16(0x807e, res);
}


TEST(modbus, WriteMultipleCoils) {
	MBErrorCode status = mb.init(&transport, &timerAdapter, 0x11);
	status = mb.enable(); 	//� �// Enable the Modbus Protocol Stack.
	mb.instance().handleTimerT35Expired();
	// Request
	transport.spyReceiveByte(0x11); // Device address
	mb.poll();
	transport.spyReceiveByte(0x0F); // Function code
	mb.poll();
	transport.spyReceiveByte(0x00); // First register address Hi byte
	mb.poll();
	transport.spyReceiveByte(0x13); // First register address Lo byte
	mb.poll();
	transport.spyReceiveByte(0x00); // Registers count Hi byte
	mb.poll();
	transport.spyReceiveByte(0x0A); // Registers count Lo byte
	mb.poll();
	transport.spyReceiveByte(0x02); // Count of next bytes
	mb.poll();
	transport.spyReceiveByte(0xCD); // Value of DO 27-20 byte (1100 1101)
	mb.poll();
	transport.spyReceiveByte(0x01); // Value of DO 29-28 byte (0000 0001)
	mb.poll();
	transport.spyReceiveByte(0xBF); // CRC
	mb.poll();
	transport.spyReceiveByte(0x0B); // CRC
	mb.poll();
	mb.instance().handleTimerT35Expired();
	mb.poll();

	for (int i = 0; i < 8; i++) {
		mb.poll();
		// Here we simply count the number of poll cycles.
	}

	u08 slaveResponse[8] = { 0x11,	 // Device address
	                         0x0F,	 // Function code
	                         0x00,	 // First register address Hi byte
	                         0x13,	 // First register address Lo byte
	                         0x00,	 // Count of registers that was wrote Hi byte
	                         0x0A,	 // Count of registers that was wrote Lo byte
	                         0x26,	 // CRC
	                         0x99
	                       }; // CRC
	checkSendedBytes(8, slaveResponse);
}

TEST(modbus, ReadMultipleCoils) {
	MBErrorCode status = mb.init(&transport, &timerAdapter, 0x11);
	status = mb.enable(); 	//� � // Enable the Modbus Protocol Stack.
	mb.instance().handleTimerT35Expired();
	ModbusRegisters::usRegCoilsBuf[20 - 1] = true;
	ModbusRegisters::usRegCoilsBuf[21 - 1] = false;
	ModbusRegisters::usRegCoilsBuf[22 - 1] = true;
	ModbusRegisters::usRegCoilsBuf[23 - 1] = true;
	ModbusRegisters::usRegCoilsBuf[24 - 1] = false;
	ModbusRegisters::usRegCoilsBuf[25 - 1] = false;
	ModbusRegisters::usRegCoilsBuf[26 - 1] = true;
	ModbusRegisters::usRegCoilsBuf[27 - 1] = true;
	ModbusRegisters::usRegCoilsBuf[28 - 1] = true;
	ModbusRegisters::usRegCoilsBuf[29 - 1] = true;
	ModbusRegisters::usRegCoilsBuf[30 - 1] = false;
	ModbusRegisters::usRegCoilsBuf[31 - 1] = true;
	ModbusRegisters::usRegCoilsBuf[32 - 1] = false;
	ModbusRegisters::usRegCoilsBuf[33 - 1] = true;
	ModbusRegisters::usRegCoilsBuf[34 - 1] = true;
	ModbusRegisters::usRegCoilsBuf[35 - 1] = false;
	ModbusRegisters::usRegCoilsBuf[36 - 1] = false;
	ModbusRegisters::usRegCoilsBuf[37 - 1] = true;
	ModbusRegisters::usRegCoilsBuf[38 - 1] = false;
	ModbusRegisters::usRegCoilsBuf[39 - 1] = false;
	ModbusRegisters::usRegCoilsBuf[40 - 1] = true;
	ModbusRegisters::usRegCoilsBuf[41 - 1] = true;
	ModbusRegisters::usRegCoilsBuf[42 - 1] = false;
	ModbusRegisters::usRegCoilsBuf[43 - 1] = true;
	ModbusRegisters::usRegCoilsBuf[44 - 1] = false;
	ModbusRegisters::usRegCoilsBuf[45 - 1] = true;
	ModbusRegisters::usRegCoilsBuf[46 - 1] = true;
	ModbusRegisters::usRegCoilsBuf[47 - 1] = true;
	ModbusRegisters::usRegCoilsBuf[48 - 1] = false;
	ModbusRegisters::usRegCoilsBuf[49 - 1] = false;
	ModbusRegisters::usRegCoilsBuf[50 - 1] = false;
	ModbusRegisters::usRegCoilsBuf[51 - 1] = false;
	ModbusRegisters::usRegCoilsBuf[52 - 1] = true;
	ModbusRegisters::usRegCoilsBuf[53 - 1] = true;
	ModbusRegisters::usRegCoilsBuf[54 - 1] = false;
	ModbusRegisters::usRegCoilsBuf[55 - 1] = true;
	ModbusRegisters::usRegCoilsBuf[56 - 1] = true;
	// Request
	transport.spyReceiveByte(0x11); // Device address
	mb.poll();
	transport.spyReceiveByte(0x01); // Function code
	mb.poll();
	transport.spyReceiveByte(0x00); // First register address Hi byte
	mb.poll();
	transport.spyReceiveByte(0x13); // First register address Lo byte
	mb.poll();
	transport.spyReceiveByte(0x00); // Registers count Hi byte
	mb.poll();
	transport.spyReceiveByte(0x25); // Registers count Lo byte
	mb.poll();
	transport.spyReceiveByte(0x0E); // CRC
	mb.poll();
	transport.spyReceiveByte(0x84); // CRC
	mb.poll();
	mb.instance().handleTimerT35Expired();
	mb.poll();

	for (int i = 0; i < 11; i++) {
		mb.poll();
		//Here we simply count the number of poll cycles.
	}

	u08 slaveResponse[10] = { 0x11,	  // Device address
	                          0x01,	  // Function code
	                          0x05,	  // Count of next bytes
	                          0xCD,	  // Value of DO 27-20 registers (1100 1101)
	                          0x6B,	  // Value of DO 35-28 registers (0110 1011)
	                          0xB2,	  // Value of DO 43-36 registers (1011 0010)
	                          0x0E,	  // Value of DO 51-44 registers (0000 1110)
	                          0x1B,   // Value of DO 56-52 registers (0001 1011)
	                          0x45,   // CRC
	                          0xE6
	                        }; // CRC
	checkSendedBytes(10, slaveResponse);
}

TEST(modbus, ReadMultipleHoldingRegisters) {
	MBErrorCode status = mb.init(&transport, &timerAdapter, 0x11);
	status = mb.enable(); 	//� � // Enable the Modbus Protocol Stack.
	mb.instance().handleTimerT35Expired();
	ModbusRegisters::usRegHoldingBuf[0x6B] = 0xAE41;
	ModbusRegisters::usRegHoldingBuf[0x6C] = 0x5652;
	ModbusRegisters::usRegHoldingBuf[0x6D] = 0x4340;
	// Request
	transport.spyReceiveByte(0x11); // Device address
	mb.poll();
	transport.spyReceiveByte(0x03); // Function code
	mb.poll();
	transport.spyReceiveByte(0x00); // First register address Hi byte
	mb.poll();
	transport.spyReceiveByte(0x6B); // First register address Lo byte
	mb.poll();
	transport.spyReceiveByte(0x00); // Registers count Hi byte
	mb.poll();
	transport.spyReceiveByte(0x03); // Registers count Lo byte
	mb.poll();
	transport.spyReceiveByte(0x76); // CRC
	mb.poll();
	transport.spyReceiveByte(0x87); // CRC
	mb.poll();
	mb.instance().handleTimerT35Expired();
	mb.poll();

	for (int i = 0; i < 12; i++) {
		mb.poll();
		// Here we simply count the number of poll cycles.
	}

	u08 slaveResponse[11] = { 0x11,	  // Device address
	                          0x03,	  // Function code
	                          0x06,	  // Count of next bytes
	                          0xAE,	  // Value of 107 register Hi
	                          0x41,	  // Value of 107 register Lo
	                          0x56,	  // Value of 108 register Hi
	                          0x52,	  // Value of 108 register Lo
	                          0x43,   // Value of 109 register Hi
	                          0x40,   // Value of 109 register Lo
	                          0x49,   // CRC
	                          0xAD
	                        }; // CRC
	checkSendedBytes(11, slaveResponse);
}

TEST(modbus, ReadSingleInputRegister) {
	MBErrorCode status = mb.init(&transport, &timerAdapter, 0x11);
	status = mb.enable(); 	//� � // Enable the Modbus Protocol Stack.
	mb.instance().handleTimerT35Expired();
	// Set registers
	ModbusRegisters::usRegInputBuf[0x08] = 0x000A;
	// Request
	transport.spyReceiveByte(0x11); // Device address
	mb.poll();
	transport.spyReceiveByte(0x04); // Function code
	mb.poll();
	transport.spyReceiveByte(0x00); // First register address Hi byte
	mb.poll();
	transport.spyReceiveByte(0x08); // First register address Lo byte
	mb.poll();
	transport.spyReceiveByte(0x00); // Registers count Hi byte
	mb.poll();
	transport.spyReceiveByte(0x01); // Registers count Lo byte
	mb.poll();
	transport.spyReceiveByte(0xB2); // CRC
	mb.poll();
	transport.spyReceiveByte(0x98); // CRC
	mb.poll();
	mb.instance().handleTimerT35Expired();
	mb.poll();

	for (int i = 0; i < 8; i++) {
		mb.poll();
		// Here we simply count the number of poll cycles.
	}

	u08 slaveResponse[7] = {  0x11,	  // Device address
	                          0x04,	  // Function code
	                          0x02,	  // Count of next bytes
	                          0x00,	  // Value of 8 register Hi
	                          0x0A,	  // Value of 8 register Lo
	                          0xF8,	  // CRC
	                          0xF4
	                       }; // CRC
	checkSendedBytes(7, slaveResponse);
}
TEST(modbus, ReadSingleHoldingRegister2) {
	MBErrorCode status = mb.init(&transport, &timerAdapter, 0x04);
	status = mb.enable(); 	//� � // Enable the Modbus Protocol Stack.
	mb.instance().handleTimerT35Expired();
	// Set registers
	ModbusRegisters::usRegHoldingBuf[0x01] = 0x000A;
	// Request
	transport.spyReceiveByte(0x04); // Device address
	mb.poll();
	transport.spyReceiveByte(0x03); // Function code
	mb.poll();
	transport.spyReceiveByte(0x00); // First register address Hi byte
	mb.poll();
	transport.spyReceiveByte(0x01); // First register address Lo byte
	mb.poll();
	transport.spyReceiveByte(0x00); // Registers count Hi byte
	mb.poll();
	transport.spyReceiveByte(0x01); // Registers count Lo byte
	mb.poll();
	transport.spyReceiveByte(0xd5); // CRC
	mb.poll();
	transport.spyReceiveByte(0x9f); // CRC
	mb.poll();
	mb.instance().handleTimerT35Expired();
	mb.poll();

	for (int i = 0; i < 8; i++) {
		mb.poll();
		// Here we simply count the number of poll cycles.
	}

	u08 slaveResponse[7] = { 0x4,	  // Device address
	                         0x03,	  // Function code
	                         0x02,	  // Count of next bytes
	                         0x00,	  // Value of 8 register Hi
	                         0x0A,	  // Value of 8 register Lo
	                         0xF4,	  // CRC
	                         0x43
	                       }; // CRC
	checkSendedBytes(7, slaveResponse);
}

TEST(modbus, WriteSingleCoil) {
	MBErrorCode status = mb.init(&transport, &timerAdapter, 0x11);
	status = mb.enable(); 	//� � // Enable the Modbus Protocol Stack.
	mb.instance().handleTimerT35Expired();
	// Request
	transport.spyReceiveByte(0x11); // Device address
	mb.poll();
	transport.spyReceiveByte(0x05); // Function code
	mb.poll();
	transport.spyReceiveByte(0x00); // First register address Hi byte
	mb.poll();
	transport.spyReceiveByte(0xAC); // First register address Lo byte
	mb.poll();
	transport.spyReceiveByte(0xFF); // Value Hi byte
	mb.poll();
	transport.spyReceiveByte(0x00); // Value Lo byte
	mb.poll();
	transport.spyReceiveByte(0x4E); // CRC
	mb.poll();
	transport.spyReceiveByte(0x8B); // CRC
	mb.poll();
	mb.instance().handleTimerT35Expired();
	mb.poll();

	for (int i = 0; i < 9; i++) {
		mb.poll();
		// Here we simply count the number of poll cycles.
	}

	u08 slaveResponse[8] = { 0x11,	  // Device address
	                         0x05,	  // Function code
	                         0x00,	  // First register address Hi byte
	                         0xAC,	  // First register address Lo byte
	                         0xFF,	  // Value Hi byte
	                         0x00,	  // Value Lo byte
	                         0x4E,	  // CRC
	                         0x8B
	                       };  // CRC
	checkSendedBytes(8, slaveResponse);
}

TEST(modbus, WriteSingleHoldingRegister) {
	MBErrorCode status = mb.init(&transport, &timerAdapter, 0x11);
	status = mb.enable(); 	//� � // Enable the Modbus Protocol Stack.
	mb.instance().handleTimerT35Expired();
	// Clear register
	ModbusRegisters::usRegHoldingBuf[0x01] = 0;
	// Request
	transport.spyReceiveByte(0x11); // Device address
	mb.poll();
	transport.spyReceiveByte(0x06); // Function code
	mb.poll();
	transport.spyReceiveByte(0x00); // First register address Hi byte
	mb.poll();
	transport.spyReceiveByte(0x01); // First register address Lo byte
	mb.poll();
	transport.spyReceiveByte(0x00); // Value Hi byte
	mb.poll();
	transport.spyReceiveByte(0x03); // Value Lo byte
	mb.poll();
	transport.spyReceiveByte(0x9A); // CRC
	mb.poll();
	transport.spyReceiveByte(0x9B); // CRC
	mb.poll();
	mb.instance().handleTimerT35Expired();
	mb.poll();

	for (int i = 0; i < 9; i++) {
		mb.poll();
		// Here we simply count the number of poll cycles.
	}

	u08 slaveResponse[8] = { 0x11,	  // Device address
	                         0x06,	  // Function code
	                         0x00,	  // First register address Hi byte
	                         0x01,	  // First register address Lo byte
	                         0x00,	  // Value Hi byte
	                         0x03,	  // Value Lo byte
	                         0x9A,	  // CRC
	                         0x9B
	                       };  // CRC
	checkSendedBytes(8, slaveResponse);
	TEST_ASSERT_EQUAL_INT(0x03, ModbusRegisters::usRegHoldingBuf[0x01]);
}

TEST(modbus, WriteMultipleHoldingRegisters) {
	MBErrorCode status = mb.init(&transport, &timerAdapter, 0x11);
	status = mb.enable(); 	//� � // Enable the Modbus Protocol Stack.
	mb.instance().handleTimerT35Expired();
	// Clear register
	ModbusRegisters::usRegHoldingBuf[0x00] = 0;
	ModbusRegisters::usRegHoldingBuf[0x01] = 0;
	ModbusRegisters::usRegHoldingBuf[0x02] = 0;
	ModbusRegisters::usRegHoldingBuf[0x03] = 0;
	ModbusRegisters::usRegHoldingBuf[0x04] = 0;
	// Request
	transport.spyReceiveByte(0x11); // Device address
	mb.poll();
	transport.spyReceiveByte(0x10); // Function code
	mb.poll();
	transport.spyReceiveByte(0x00); // First register address Hi byte
	mb.poll();
	transport.spyReceiveByte(0x01); // First register address Lo byte
	mb.poll();
	transport.spyReceiveByte(0x00); // Register count Hi byte
	mb.poll();
	transport.spyReceiveByte(0x02); // Register count Lo byte
	mb.poll();
	transport.spyReceiveByte(0x04); // Count of next bytes
	mb.poll();
	transport.spyReceiveByte(0x00); // Value Hi reg. 0x02
	mb.poll();
	transport.spyReceiveByte(0x0A); // Value Lo reg. 0x02
	mb.poll();
	transport.spyReceiveByte(0x01); // Value Hi reg. 0x03
	mb.poll();
	transport.spyReceiveByte(0x02); // Value Lo reg. 0x03
	mb.poll();
	transport.spyReceiveByte(0xC6); // CRC
	mb.poll();
	transport.spyReceiveByte(0xF0); // CRC
	mb.poll();
	mb.instance().handleTimerT35Expired();
	mb.poll();

	for (int i = 0; i < 9; i++) {
		mb.poll();
		// Here we simply count the number of poll cycles.
	}

	u08 slaveResponse[8] = { 0x11,	  // Device address
	                         0x10,	  // Function code
	                         0x00,	  // First register address Hi byte
	                         0x01,	  // First register address Lo byte
	                         0x00,	  // Count of written registers Hi byte
	                         0x02,	  // Count of written registers Lo byte
	                         0x12,	  // CRC
	                         0x98
	                       };  // CRC
	checkSendedBytes(8, slaveResponse);
	TEST_ASSERT_EQUAL_INT(0x0000, ModbusRegisters::usRegHoldingBuf[0x00]);
	TEST_ASSERT_EQUAL_INT(0x000A, ModbusRegisters::usRegHoldingBuf[0x01]);
	TEST_ASSERT_EQUAL_INT(0x0102, ModbusRegisters::usRegHoldingBuf[0x02]);
	TEST_ASSERT_EQUAL_INT(0x0000, ModbusRegisters::usRegHoldingBuf[0x03]);
	TEST_ASSERT_EQUAL_INT(0x0000, ModbusRegisters::usRegHoldingBuf[0x04]);
}

TEST(modbus, ReadMultipleDiscreteInputs) {
	MBErrorCode status = mb.init(&transport, &timerAdapter, 0x11);
	status = mb.enable(); 	//� � // Enable the Modbus Protocol Stack.
	mb.instance().handleTimerT35Expired();
	ModbusRegisters::usRegDiscreteBuf[196] = false;
	ModbusRegisters::usRegDiscreteBuf[197] = false;
	ModbusRegisters::usRegDiscreteBuf[198] = true;
	ModbusRegisters::usRegDiscreteBuf[199] = true;
	ModbusRegisters::usRegDiscreteBuf[200] = false;
	ModbusRegisters::usRegDiscreteBuf[201] = true;
	ModbusRegisters::usRegDiscreteBuf[202] = false;
	ModbusRegisters::usRegDiscreteBuf[203] = true;
	ModbusRegisters::usRegDiscreteBuf[204] = true;
	ModbusRegisters::usRegDiscreteBuf[205] = true;
	ModbusRegisters::usRegDiscreteBuf[206] = false;
	ModbusRegisters::usRegDiscreteBuf[207] = true;
	ModbusRegisters::usRegDiscreteBuf[208] = true;
	ModbusRegisters::usRegDiscreteBuf[209] = false;
	ModbusRegisters::usRegDiscreteBuf[210] = true;
	ModbusRegisters::usRegDiscreteBuf[211] = true;
	ModbusRegisters::usRegDiscreteBuf[212] = true;
	ModbusRegisters::usRegDiscreteBuf[213] = false;
	ModbusRegisters::usRegDiscreteBuf[214] = true;
	ModbusRegisters::usRegDiscreteBuf[215] = false;
	ModbusRegisters::usRegDiscreteBuf[216] = true;
	ModbusRegisters::usRegDiscreteBuf[217] = true;
	ModbusRegisters::usRegDiscreteBuf[218] = true;
	ModbusRegisters::usRegDiscreteBuf[219] = true;
	// Request
	transport.spyReceiveByte(0x11); // Device address
	mb.poll();
	transport.spyReceiveByte(0x02); // Function code
	mb.poll();
	transport.spyReceiveByte(0x00); // First register address Hi byte
	mb.poll();
	transport.spyReceiveByte(0xC4); // First register address Lo byte
	mb.poll();
	transport.spyReceiveByte(0x00); // Registers count Hi byte
	mb.poll();
	transport.spyReceiveByte(0x16); // Registers count Lo byte
	mb.poll();
	transport.spyReceiveByte(0xBA); // CRC
	mb.poll();
	transport.spyReceiveByte(0xA9); // CRC
	mb.poll();
	mb.instance().handleTimerT35Expired();
	mb.poll();

	for (int i = 0; i < 9; i++) {
		mb.poll();
		// Here we simply count the number of poll cycles.
	}

	u08 slaveResponse[8] = { 0x11,	  // Device address
	                         0x02,	  // Function code
	                         0x03,	  // Count of next bytes
	                         0xAC,	  // Value of DO 203-196 registers (1010 1100)
	                         0xDB,	  // Value of DO 211-204 registers (1101 1011)
	                         0x35,	  // Value of DO 219-212 registers (0011 0101)
	                         0x20,   // CRC
	                         0x18
	                       }; // CRC
	checkSendedBytes(8, slaveResponse);
}



void someDataExchange(void) { // ReadSingleInputRegister actualy
	// Set registers
	ModbusRegisters::usRegInputBuf[0x08] = 0x000A;
	// Request
	transport.spyReceiveByte(0x11); // Device address
	mb.poll();
	transport.spyReceiveByte(0x04); // Function code
	mb.poll();
	transport.spyReceiveByte(0x00); // First register address Hi byte
	mb.poll();
	transport.spyReceiveByte(0x08); // First register address Lo byte
	mb.poll();
	transport.spyReceiveByte(0x00); // Registers count Hi byte
	mb.poll();
	transport.spyReceiveByte(0x01); // Registers count Lo byte
	mb.poll();
	transport.spyReceiveByte(0xB2); // CRC
	mb.poll();
	transport.spyReceiveByte(0x98); // CRC
	mb.poll();
	mb.instance().handleTimerT35Expired();
	mb.poll();

	for (int i = 0; i < 8; i++) {
		mb.poll();
		// Here we simply count the number of poll cycles.
	}

	u08 slaveResponse[7] = { 0x11,	  // Device address
	                         0x04,	  // Function code
	                         0x02,	  // Count of next bytes
	                         0x00,	  // Value of 8 register Hi
	                         0x0A,	  // Value of 8 register Lo
	                         0xF8,	  // CRC
	                         0xF4
	                       }; // CRC
	checkSendedBytes(7, slaveResponse);
}

TEST(modbus, ChangeModbusAddress) {
	MBErrorCode status = mb.init(&transport, &timerAdapter, 0x11);
	status = mb.enable(); 	//� � // Enable the Modbus Protocol Stack.
	mb.instance().handleTimerT35Expired();
	status = mb.init(&transport, &timerAdapter, 0x11);
	status = mb.enable(); 	//� � // Enable the Modbus Protocol Stack.
	mb.instance().handleTimerT35Expired();
	someDataExchange();
}



TEST(modbus, WrongCoilAddressError) {
	MBErrorCode status = mb.init(&transport, &timerAdapter, 0x0A);
	status = mb.enable(); 	//� � //Enable the Modbus Protocol Stack.
	mb.instance().handleTimerT35Expired();
	// Request
	transport.spyReceiveByte(0x0A); // Device address
	mb.poll();
	transport.spyReceiveByte(0x01); // Function code
	mb.poll();
	transport.spyReceiveByte(0x04); // First register address Hi byte
	mb.poll();
	transport.spyReceiveByte(0xA1); // First register address Lo byte
	mb.poll();
	transport.spyReceiveByte(0x00); // Registers count Hi byte
	mb.poll();
	transport.spyReceiveByte(0x01); // Registers count Lo byte
	mb.poll();
	transport.spyReceiveByte(0xAC); // CRC Lo
	mb.poll();
	transport.spyReceiveByte(0x63); // CRC Hi
	mb.poll();
	mb.instance().handleTimerT35Expired();
	mb.poll();

	for (int i = 0; i < 9; i++) {
		mb.poll();
		// Here we simply count the number of poll cycles.
	}

	u08 slaveResponse[5] = { 0x0A,	  // Device address
	                         0x81,	  // Function code with changed bit
	                         0x02,	  // Error code
	                         0xB0,	  // CRC
	                         0x53
	                       }	; // CRC
	checkSendedBytes(5, slaveResponse);
}



TEST_GROUP_RUNNER(modbus) {
	RUN_TEST_CASE(modbus, TestCRC);
	RUN_TEST_CASE(modbus, WriteMultipleCoils);
	RUN_TEST_CASE(modbus, ReadMultipleCoils);
	RUN_TEST_CASE(modbus, ReadMultipleHoldingRegisters);
	RUN_TEST_CASE(modbus, ReadSingleInputRegister);
	RUN_TEST_CASE(modbus, ReadSingleHoldingRegister2);
	RUN_TEST_CASE(modbus, WriteSingleCoil);
	RUN_TEST_CASE(modbus, WriteSingleHoldingRegister);
	RUN_TEST_CASE(modbus, WriteMultipleHoldingRegisters);
	RUN_TEST_CASE(modbus, ReadMultipleDiscreteInputs);
	RUN_TEST_CASE(modbus, ChangeModbusAddress);
	RUN_TEST_CASE(modbus, WrongCoilAddressError);
}
