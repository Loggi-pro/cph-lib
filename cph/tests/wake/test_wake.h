#pragma once
#include <unity/unity_fixture_classes.h>
#include <cph/wake/wake.h>
#include <cph/wake/crc.h>


template <int ID>
class CphTest_Wake : UnityTestClass<CphTest_Wake<ID>> {
		static inline const s16 _WAKE_ADDR = 1;
		static inline uint8_t _buffer[255];
		static inline const cph::TWakePacket _packet = cph::TWakePacket(_buffer);
		typedef cph::TWakeProtocol TWake;


		static void _WakeSetIncomingData(vd::gsl::span<uint8_t> data) {
			for (uint16_t i = 0; i < data.size(); i++) {
				TWake::handleData(data[i]);
			}
		}

		static void wakePacketInitialize(void) {
			for (uint8_t i = 0; i < sizeof(_buffer); ++i) {
				_buffer[i] = 0;
			}
		}
		static void staffByteIfneeded(u08* array, u16* index, u08 byte) {
			if (byte == cph::TWakeInfo::FEND) {
				array[(*index)++] = cph::TWakeInfo::FESC;
				array[(*index)++] = cph::TWakeInfo::TFEND;
				return;
			}

			if (byte == cph::TWakeInfo::FESC) {
				array[(*index)++] = cph::TWakeInfo::FESC;
				array[(*index)++] = cph::TWakeInfo::TFESC;
				return;
			}

			array[(*index)++] = byte;
		}
		static void _makeArray(u08* expectationArray, u16* expectationArraySize,
		                       uint16_t address, u08 command,
		                       u08 dataBytesCount, u08* dataBytes) {
			u16 index = 0;
			u08 crc;
			u08 const startByte = cph::TWakeInfo::FEND;
			expectationArray[index++] = startByte;
			cph::TWakeAddress addr(address);
			crc = cph::TWakeInfo::weCRC_INIT;
			Calc_crc8(&crc, cph::TWakeInfo::weCRC_POLY, startByte);

			if (address < 128) {
				staffByteIfneeded(expectationArray, &index, addr.getHiPart() | 0x80);
				Calc_crc8(&crc, cph::TWakeInfo::weCRC_POLY, addr.getHiPart());
			} else {
				staffByteIfneeded(expectationArray, &index, addr.getLowPart() | 0x80);
				staffByteIfneeded(expectationArray, &index, addr.getHiPart() | 0x80);
				Calc_crc8(&crc, cph::TWakeInfo::weCRC_POLY, addr.getLowPart());
				Calc_crc8(&crc, cph::TWakeInfo::weCRC_POLY, addr.getHiPart());
			}

			expectationArray[index++] = command & ~(0x80);
			staffByteIfneeded(expectationArray, &index, dataBytesCount);

			for (u08 i = 0; i < dataBytesCount; i++) {
				staffByteIfneeded(expectationArray, &index, dataBytes[i]);
			}

			Calc_crc8(&crc, cph::TWakeInfo::weCRC_POLY, command);
			Calc_crc8(&crc, cph::TWakeInfo::weCRC_POLY, dataBytesCount);

			for (u08 i = 0; i < dataBytesCount; i++) {
				Calc_crc8(&crc, cph::TWakeInfo::weCRC_POLY, dataBytes[i]);
			}

			staffByteIfneeded(expectationArray, &index, crc);
			*expectationArraySize = index;
		}

		static void _WakeSetIncomingPacket(u08 startByte, uint16_t address, u08 command,
		                                   u08 dataBytesCount,
		                                   u08* dataBytes) {
			u08 expectationArray[270];
			u16 expectationArraySize;
			_makeArray(expectationArray, &expectationArraySize, address, command, dataBytesCount,
			           dataBytes);
			_WakeSetIncomingData(vd::gsl::span<uint8_t>(expectationArray, expectationArraySize));
		}



	public:
		static void setup() {
			wakePacketInitialize();
			TWake::init<255>(cph::TWakeAddress(_WAKE_ADDR));
			TWake::reset();
		}
		static void tearDown() {
			TWake::reset();

			for (int i = 0; i < COUNT_OF(_buffer); ++i) {
				_buffer[i] = 0;
			}
		}

		static void Wake_CrcCalculationIsCorrect() {
			const u08 crcInitialValue = 0xDE;
			const u08 crcPolinonm = 0x31;
			u08 crc = crcInitialValue;
			Calc_crc8(&crc, crcPolinonm, cph::TWakeInfo::FEND);
			Calc_crc8(&crc, crcPolinonm, 0x03);
			Calc_crc8(&crc, crcPolinonm, 0xA5);
			TEST_ASSERT_EQUAL_INT(0x77, crc);
			// For crc calculation online calculator is used
			// http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
		}






		static void  Wake_WakeInitializationConfigUart() {
			TWake::deinit();
			TWake::init<255>(cph::TWakeAddress(_WAKE_ADDR));
		}

		/* FEND|ADDR|CMD| N|DATA|DATA|DATA|DATA|DATA|CRC|
		   FEND|129+|  1| 0|    |    |    |    |    |CRC|*/
		static void Wake_ReceivedGlobalCommandNoData() {
			uint8_t data[] = { cph::TWakeInfo::FEND, cph::TWakeInfo::weBROADCAST | 0x80, 0x02, 0x00, 0xC8 };
			_WakeSetIncomingData(data);

			if (TWake::readPacketAsync(_buffer)) {
				TEST_ASSERT_EQUAL_INT(2, _packet.cmd());
			} else {
				TEST_FAIL();
			}
		}

		static void Wake_ReceivedWrongCRCMessageIgnored() {
			uint8_t data[] = { cph::TWakeInfo::FEND, cph::TWakeInfo::weBROADCAST | 0x80, 0x02, 0x00, 0x23 };
			_WakeSetIncomingData(data);

			if (TWake::readPacketAsync(_buffer)) {
				TEST_FAIL();
			}
		}

		static void Wake_ReceivedOnlyCommand() {
			uint8_t data[] = { cph::TWakeInfo::FEND, 0x02, 0x00, 0xFE };
			_WakeSetIncomingData(data);

			if (TWake::readPacketAsync(_buffer)) {
				TEST_ASSERT_EQUAL_INT(2, _packet.cmd());
			} else {
				TEST_FAIL();
			}
		}

		static void Wake_ReceivedOnlyCommandNoDataByteCount() {
			uint8_t data[] = { cph::TWakeInfo::FEND, 0x03, 0xF9 };
			_WakeSetIncomingData(data);

			if (TWake::readPacketAsync(_buffer)) {
				TEST_FAIL();
			}
		}

		static void Wake_ReceivedWrongMessageStartByte() {
			uint8_t data[] = { cph::TWakeInfo::FEND + 1, cph::TWakeInfo::weBROADCAST | 0x80, 0x02, 0x00, 0x53 };
			_WakeSetIncomingData(data);

			if (TWake::readPacketAsync(_buffer)) {
				TEST_FAIL();
			}
		}

		static void Wake_ReceivedWrongMessageAndThenRight() {
			// Message with wrong CRC
			uint8_t data[] = { cph::TWakeInfo::FEND, cph::TWakeInfo::weBROADCAST | 0x80, 0x02, 0x00, 0x53 };
			_WakeSetIncomingData(data);

			if (TWake::readPacketAsync(_buffer)) {
				TEST_FAIL();
			}

			// Correct message
			uint8_t data2[] = { cph::TWakeInfo::FEND, cph::TWakeInfo::weBROADCAST | 0x80, 0x02, 0x00, 0xC8 };
			_WakeSetIncomingData(data2);

			if (TWake::readPacketAsync(_buffer)) {
				TEST_ASSERT_EQUAL_INT(2, _packet.cmd());
			} else {
				TEST_FAIL();
			}
		}

		static void Wake_ReceivedOneDataByte() {
			uint8_t data[] = { cph::TWakeInfo::FEND, cph::TWakeInfo::weBROADCAST | 0x80, 0x00, 0x01, 0xA2, 0x18 };
			_WakeSetIncomingData(data);

			if (TWake::readPacketAsync(_buffer)) {
				TEST_ASSERT_EQUAL_INT(0, _packet.cmd());
				TEST_ASSERT_EQUAL_INT(1, _packet.count());
				TEST_ASSERT_EQUAL_INT(0xA2, _packet.data()[0]);
			} else {
				TEST_FAIL();
			}
		}

		static void Wake_ReceivedFewDataByte() {
			u08 dataBytes[5] = { 0, 1, 2, 3, 4 };
			_WakeSetIncomingPacket(cph::TWakeInfo::FEND, cph::TWakeInfo::weBROADCAST, 0, 5, dataBytes);

			if (TWake::readPacketAsync(_buffer)) {
				TEST_ASSERT_EQUAL_INT(0, _packet.cmd());
				TEST_ASSERT_EQUAL_INT(5, _packet.count());
				TEST_ASSERT_EQUAL_INT(0x00, _packet.data()[0]);
				TEST_ASSERT_EQUAL_INT(0x01, _packet.data()[1]);
				TEST_ASSERT_EQUAL_INT(0x02, _packet.data()[2]);
				TEST_ASSERT_EQUAL_INT(0x03, _packet.data()[3]);
				TEST_ASSERT_EQUAL_INT(0x04, _packet.data()[4]);
			} else {
				TEST_FAIL();
			}
		}

		static void Wake_ReceivedMaximumDataBytes() {
			u08 dataBytes[255 - 2];

			for (u08 i = 0; i < 255 - 2; i++) {
				dataBytes[i] = i;
			}

			_WakeSetIncomingPacket(cph::TWakeInfo::FEND, cph::TWakeInfo::weBROADCAST, 0, 255 - 2, dataBytes);

			if (TWake::readPacketAsync(_buffer)) {
				TEST_ASSERT_EQUAL_INT(0, _packet.cmd());
				TEST_ASSERT_EQUAL_INT(255 - 2, _packet.count());

				for (u08 i = 0; i < 255 - 2; i++) {
					TEST_ASSERT_EQUAL_INT(i, _packet.data()[i]);
				}
			} else {
				TEST_FAIL();
			}
		}

		static void Wake_ReceivedRightAddress() {
			uint8_t data[] = { cph::TWakeInfo::FEND, _WAKE_ADDR | 0x80, 0x00, 0x01, 0xA2, 0x83 };
			_WakeSetIncomingData(data);

			if (TWake::readPacketAsync(_buffer)) {
				TEST_ASSERT_EQUAL_INT(0, _packet.cmd());
				TEST_ASSERT_EQUAL_INT(1, _packet.count());
				TEST_ASSERT_EQUAL_INT(0xA2, _packet.data()[0]);
			} else {
				TEST_FAIL();
			}
		}

		static void Wake_ReceivedTwoByteAddress() {
			TWake::init<255>(cph::TWakeAddress(129));
			uint8_t data[] = { cph::TWakeInfo::FEND, 0x01 | 0x80, 0x01 | 0x80, 0x00, 0x01, 0xA2, 0xB8 };
			//uint8_t value = 0xA2;
			//_WakeSetIncomingPacket(cph::TWakeInfo::FEND, 129, 0, 1, &value);
			_WakeSetIncomingData(data);

			if (TWake::readPacketAsync(_buffer)) {
				TEST_ASSERT_EQUAL_INT(0, _packet.cmd());
				TEST_ASSERT_EQUAL_INT(1, _packet.count());
				TEST_ASSERT_EQUAL_INT(0xA2, _packet.data()[0]);
			} else {
				TEST_FAIL();
			}
		}

		static void Wake_ReceivedTwoByteAddress_WithWrongHiByte() {
			TWake::init<255>(cph::TWakeAddress(129));
			uint8_t data[] = { cph::TWakeInfo::FEND, 0x01 | 0x80, 0x03 | 0x80, 0x00, 0x01, 0xA2, 0xBF };
			//uint8_t value = 0xA2;
			//_WakeSetIncomingPacket(cph::TWakeInfo::FEND, 385, 0, 1, &value);
			_WakeSetIncomingData(data);

			if (TWake::readPacketAsync(_buffer)) {
				TEST_FAIL();
			};

			TEST_ASSERT(TWake::getError() == cph::TWakeInfo::weADDR);
		}

		static void Wake_ReceivedWrongAddress() {
			uint8_t data[] = { cph::TWakeInfo::FEND, (_WAKE_ADDR + 1) | 0x80, 0x00, 0x01, 0xA2, 0x83 };
			_WakeSetIncomingData(data);

			if (TWake::readPacketAsync(_buffer)) {
				TEST_FAIL();
			}
		}

		static void Wake_ReceivedStuffedFendByte() {
			uint8_t data[] = { cph::TWakeInfo::FEND, cph::TWakeInfo::weBROADCAST | 0x80, 0x00, 0x01, cph::TWakeInfo::FESC, cph::TWakeInfo::TFEND, 0xC1 };
			_WakeSetIncomingData(data);

			if (TWake::readPacketAsync(_buffer)) {
				TEST_ASSERT_EQUAL_INT(0, _packet.cmd());
				TEST_ASSERT_EQUAL_INT(1, _packet.count());
				TEST_ASSERT_EQUAL_INT(cph::TWakeInfo::FEND, _packet.data()[0]);
			} else {
				TEST_FAIL();
			}
		}

		static void Wake_ReceivedStuffedFescByte() {
			uint8_t data[] = { cph::TWakeInfo::FEND, cph::TWakeInfo::weBROADCAST | 0x80, 0x00, 0x01, cph::TWakeInfo::FESC, cph::TWakeInfo::TFESC, 0x68 };
			_WakeSetIncomingData(data);

			if (TWake::readPacketAsync(_buffer)) {
				TEST_ASSERT_EQUAL_INT(0, _packet.cmd());
				TEST_ASSERT_EQUAL_INT(1, _packet.count());
				TEST_ASSERT_EQUAL_INT(cph::TWakeInfo::FESC, _packet.data()[0]);
			} else {
				TEST_FAIL();
			}
		}


		static void Wake_ReceivedTwoPackets_SecondIsBad() {
			uint8_t data[] = { cph::TWakeInfo::FEND, cph::TWakeInfo::weBROADCAST | 0x80, 0x00, 0x01, cph::TWakeInfo::FESC, cph::TWakeInfo::TFESC, 0x68 };
			uint8_t bad_data[] = { cph::TWakeInfo::FEND, cph::TWakeInfo::weBROADCAST | 0x80, 0x01, cph::TWakeInfo::FEND };
			_WakeSetIncomingData(data);
			_WakeSetIncomingData(bad_data);

			if (TWake::readPacketAsync(_buffer)) {
				TEST_ASSERT_EQUAL_INT(0, _packet.cmd());
				TEST_ASSERT_EQUAL_INT(1, _packet.count());
				TEST_ASSERT_EQUAL_INT(cph::TWakeInfo::FESC, _packet.data()[0]);
			} else {
				TEST_FAIL();
			}

			TEST_ASSERT_EQUAL(false, TWake::readPacketAsync(_buffer));
		}
		static void Wake_ReceivedThreePackets_SecondIsBad_ThirdIsGood() {
			uint8_t bad_data[] = { cph::TWakeInfo::FEND, cph::TWakeInfo::weBROADCAST | 0x80, 0x01, cph::TWakeInfo::FEND };
			u08 expectationArray1[] = { 3, 4 };
			u08 expectationArray2[] = { 1, 2, 3, 4, 5, 6 };
			_WakeSetIncomingPacket(cph::TWakeInfo::FEND, cph::TWakeInfo::weBROADCAST, 1, 2,
			                       expectationArray1);
			_WakeSetIncomingData(bad_data);
			_WakeSetIncomingPacket(cph::TWakeInfo::FEND, cph::TWakeInfo::weBROADCAST, 5, 6,
			                       expectationArray2);
			TEST_ASSERT_EQUAL_INT(2, TWake::count());
			TWake::readPacketAsync(_buffer);//read first packet

			if (TWake::readPacketAsync(_buffer)) {//read second packet
				TEST_ASSERT_EQUAL_INT(5, _packet.cmd());
				TEST_ASSERT_EQUAL_INT(6, _packet.count());
				TEST_ASSERT_EQUAL_MEMORY(expectationArray2, _packet.data(), 6);
			} else {
				TEST_FAIL();
			}

			TEST_ASSERT_EQUAL(false, TWake::readPacketAsync(_buffer));
		}

		static void Wake_SendOnlyCommand_SkipAddrByte() {
			s16 toAddress = -1;
			u08 expectationArray[4] = { cph::TWakeInfo::FEND, 3, 0, 0x0A };
			TEST_ASSERT_EQUAL(4, TWake::makePacket(_buffer, toAddress, 3).size());
			TEST_ASSERT_EQUAL_MEMORY(expectationArray, _buffer, 4);
		}
		static void Wake_SendCommandWithoutData() {
			u08 toAddress = 0;
			u08 expectationArray[5] = { cph::TWakeInfo::FEND, cph::TWakeInfo::weBROADCAST | 0x80, 0x03, 0x00, 0x3C };
			TEST_ASSERT_EQUAL(5, TWake::makePacket(_buffer, toAddress, 3, vd::gsl::span<uint8_t>()).size());
			TEST_ASSERT_EQUAL_MEMORY(expectationArray, _buffer, 5);
		}


		static void Wake_SendStaffedAddress() {
			u08 toAddress = 0x40; // augmented address will be fend = cph::TWakeInfo::FEND and must be staffed
			u08 expectationArray[6] = { cph::TWakeInfo::FEND, cph::TWakeInfo::FESC, cph::TWakeInfo::TFEND, 0x03, 0x00, 0xCE };
			TEST_ASSERT_EQUAL(6, TWake::makePacket(_buffer, toAddress, 3, vd::gsl::span<uint8_t>()).size());
			TEST_ASSERT_EQUAL_MEMORY(expectationArray, _buffer, 6);
		}

		static void Wake_SendOneDataByte() {
			u08 toAddress = 0;
			u08 expectationArray[6] = { cph::TWakeInfo::FEND, cph::TWakeInfo::weBROADCAST | 0x80, 0x00, 0x01, 0xFE, 0x1B };
			uint8_t data[1] = { 0xFE };
			TEST_ASSERT_EQUAL(6, TWake::makePacket(_buffer, toAddress, 0, data).size());
			TEST_ASSERT_EQUAL_MEMORY(expectationArray, _buffer, 6);
		}

		static void Wake_SendFewDataByte() {
			u08 toAddress = 0;
			u08 expectationArray[10] = { cph::TWakeInfo::FEND, cph::TWakeInfo::weBROADCAST | 0x80, 0x00, 0x05, 0x01, 0x02,
			                             0x03, 0x04, 0x05, 0xFD
			                           };
			uint8_t data[] = { 1, 2, 3, 4, 5 };
			TEST_ASSERT_EQUAL(10, TWake::makePacket(_buffer, toAddress, 0, data).size());
			TEST_ASSERT_EQUAL_MEMORY(expectationArray, _buffer, 10);
		}

		static void Wake_SendTwoByteAddress() {
			s16 toAddress = 0x0220; // 544 dec
			u08 expectationArray[6] = { cph::TWakeInfo::FEND, 0xA0, 0x84, 0x00, 0x00, 0x2B };
			// 0x0220 divides into two bytes 7 bit each
			// 00|00 0010 0|010 0000
			// Lo = 010 0000 = 0x20
			// Hi = 00 0010 0 = 0x04
			// Lo address byte goes first
			// 1+010 0000 = 0xA0
			// 1+00 0010 0 = 0x84
			uint8_t data[] = { 1, 2, 3, 4, 5 };
			TEST_ASSERT_EQUAL(6, TWake::makePacket(_buffer, toAddress, 0).size());
			TEST_ASSERT_EQUAL_MEMORY(expectationArray, _buffer, 6);
		}

		static void Wake_ReceivedCRCStuffed() {
			uint8_t data[] = { cph::TWakeInfo::FEND, 0x80, 0x23, 0x01, 0x05, cph::TWakeInfo::FESC, cph::TWakeInfo::TFEND };
			_WakeSetIncomingData(data);

			if (TWake::readPacketAsync(_buffer)) {
				TEST_ASSERT_EQUAL_INT(0x23, _packet.cmd());
				TEST_ASSERT_EQUAL_INT(1, _packet.count());
				TEST_ASSERT_EQUAL_INT(5, _packet.data()[0]);
			} else {
				TEST_FAIL();
			}

			u08 data2 = 5;
			_WakeSetIncomingPacket(cph::TWakeInfo::FEND, 0, 0x23, 1, &data2);

			if (TWake::readPacketAsync(_buffer)) {
				TEST_ASSERT_EQUAL_INT(0x23, _packet.cmd());
				TEST_ASSERT_EQUAL_INT(1, _packet.count());
				TEST_ASSERT_EQUAL_INT(5, _packet.data()[0]);
			} else {
				TEST_FAIL();
			}
		}


		static void Wake_ReceivedAdressStuffed() {
			TWake::init<255>(cph::TWakeAddress(cph::TWakeInfo::FEND));
			_WakeSetIncomingPacket(cph::TWakeInfo::FEND, cph::TWakeInfo::FEND, 0, 0, nullptr);

			if (!TWake::readPacketAsync(_buffer)) {
				TEST_FAIL();
			}
		}

		/*
		static bool _result = false;
		static uint8_t _data[10] = {};
		static uint8_t _cmd = 0;
		static void callback(uint8_t cmd, uint8_t count, const uint8_t* data) {
			_result = !_result;
			_cmd = cmd;

			for (uint8_t i = 0; i < count; ++i) {
				_data[i] = data[i];
			}
		}
		*/
		static void Wake_CheckCallback() {
			/*TWake::onDataReceive = &callback;
			uint8_t data[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
			uint8_t data2[] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };
			TEST_ASSERT(_result == false);
			_WakeSetIncomingPacket(cph::TWakeInfo::FEND, cph::TWakeInfo::weBROADCAST, 1, sizeof(data),
									   data);
			TEST_ASSERT(_result == true);
			TEST_ASSERT(_cmd == 1);
			TEST_ASSERT_EQUAL_MEMORY(data, _data, 10 * sizeof(uint8_t));
			//
			_WakeSetIncomingPacket(cph::TWakeInfo::FEND, cph::TWakeInfo::weBROADCAST, 2, sizeof(data2),
									   data2);
			TEST_ASSERT(_result == false);
			TEST_ASSERT(_cmd == 2);
			TEST_ASSERT_EQUAL_MEMORY(data2, _data, 10 * sizeof(uint8_t));
			*/
		}




		static void run() {
			RUN_TEST_CASE_CLASS(CphTest_Wake<ID>, Wake_CrcCalculationIsCorrect);
			RUN_TEST_CASE_CLASS(CphTest_Wake<ID>, Wake_WakeInitializationConfigUart);
			RUN_TEST_CASE_CLASS(CphTest_Wake<ID>, Wake_ReceivedGlobalCommandNoData);
			RUN_TEST_CASE_CLASS(CphTest_Wake<ID>, Wake_ReceivedWrongCRCMessageIgnored);
			RUN_TEST_CASE_CLASS(CphTest_Wake<ID>, Wake_ReceivedOnlyCommand);
			RUN_TEST_CASE_CLASS(CphTest_Wake<ID>, Wake_ReceivedOnlyCommandNoDataByteCount);
			RUN_TEST_CASE_CLASS(CphTest_Wake<ID>, Wake_ReceivedWrongMessageStartByte);
			RUN_TEST_CASE_CLASS(CphTest_Wake<ID>, Wake_ReceivedWrongMessageAndThenRight);
			RUN_TEST_CASE_CLASS(CphTest_Wake<ID>, Wake_ReceivedOneDataByte);
			RUN_TEST_CASE_CLASS(CphTest_Wake<ID>, Wake_ReceivedFewDataByte);
			RUN_TEST_CASE_CLASS(CphTest_Wake<ID>, Wake_ReceivedMaximumDataBytes);
			RUN_TEST_CASE_CLASS(CphTest_Wake<ID>, Wake_ReceivedRightAddress);
			RUN_TEST_CASE_CLASS(CphTest_Wake<ID>, Wake_ReceivedWrongAddress);
			RUN_TEST_CASE_CLASS(CphTest_Wake<ID>, Wake_ReceivedTwoByteAddress);
			RUN_TEST_CASE_CLASS(CphTest_Wake<ID>, Wake_ReceivedTwoByteAddress_WithWrongHiByte);
			RUN_TEST_CASE_CLASS(CphTest_Wake<ID>, Wake_ReceivedStuffedFendByte);
			RUN_TEST_CASE_CLASS(CphTest_Wake<ID>, Wake_ReceivedStuffedFescByte);
			RUN_TEST_CASE_CLASS(CphTest_Wake<ID>, Wake_ReceivedTwoPackets_SecondIsBad);
			RUN_TEST_CASE_CLASS(CphTest_Wake<ID>, Wake_ReceivedThreePackets_SecondIsBad_ThirdIsGood);
			RUN_TEST_CASE_CLASS(CphTest_Wake<ID>, Wake_SendOnlyCommand_SkipAddrByte);
			RUN_TEST_CASE_CLASS(CphTest_Wake<ID>, Wake_SendCommandWithoutData);
			RUN_TEST_CASE_CLASS(CphTest_Wake<ID>, Wake_SendStaffedAddress);
			RUN_TEST_CASE_CLASS(CphTest_Wake<ID>, Wake_SendOneDataByte);
			RUN_TEST_CASE_CLASS(CphTest_Wake<ID>, Wake_SendFewDataByte);
			RUN_TEST_CASE_CLASS(CphTest_Wake<ID>, Wake_SendTwoByteAddress);
			RUN_TEST_CASE_CLASS(CphTest_Wake<ID>, Wake_ReceivedCRCStuffed);
			RUN_TEST_CASE_CLASS(CphTest_Wake<ID>, Wake_ReceivedAdressStuffed);
			RUN_TEST_CASE_CLASS(CphTest_Wake<ID>, Wake_CheckCallback);
		}
};





