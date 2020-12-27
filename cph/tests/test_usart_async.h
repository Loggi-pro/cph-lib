#pragma once
#include <unity/unity_fixture_classes.h>
#include <cph/basic_usart.h>

#ifdef UART_RECEIVE_BYTE
	#undef UART_RECEIVE_BYTE
#endif
#ifdef UART_RECEIVE_ARRAY
	#undef UART_RECEIVE_ARRAY
#endif
#ifdef UART_CHECK_SENDED
	#undef UART_CHECK_SENDED
#endif
#define UART_RECEIVE_BYTE(byte) {TUsart::Spy::setReceiveByte(byte); /*TUsart::IntRxHandler();*/ }
#define UART_RECEIVE_ARRAY(arr,count) for (uint16_t i=0;i<count;++i){TUsart::Spy::setReceiveByte(arr[i]);/* TUsart::IntRxHandler();*/ }
#define UART_CHECK_SENDED(arr,count) for (uint16_t i=0;i<count;++i){\
		u08 _byte;\
		TEST_ASSERT_TRUE(TUsart::Spy::getSendedByte(&_byte));\
		TEST_ASSERT_EQUAL_INT(arr[i],_byte);\
	}


class CphTest_AsyncUsart : UnityTestClass<CphTest_AsyncUsart> {
		typedef cph::UsartAsync<cph::UsartFake> TUsart;
	public:
		static void setup() {
			TUsart::init<19200>();
		}
		static void tearDown() {
			TUsart::Spy::clearOutBuffer();
			TUsart::deinit();
		}

		static void Uart_Init_Default() {
			TEST_ASSERT_TRUE(TUsart::Spy::isInited());
			TEST_ASSERT_EQUAL_INT32(19200, TUsart::Spy::getBaudrate());
			TEST_ASSERT_EQUAL_INT(TUsart::Flags::DataBits8, TUsart::Spy::getDataBits());
			TEST_ASSERT_EQUAL_INT(TUsart::Flags::OneStopBit, TUsart::Spy::getStopBits());
			TEST_ASSERT_EQUAL_INT(TUsart::Flags::NoneParity, TUsart::Spy::getParity());
			TEST_ASSERT_EQUAL_INT(TUsart::Flags::RxTxEnable, TUsart::Spy::getMode());
		}

		static void Uart_Init() {
			TUsart::deinit();
			TUsart::init<115200, TUsart::Flags::DataBits5, TUsart::Flags::HalfStopBit, TUsart::Flags::OddParity, TUsart::Flags::RxEnable>();
			TEST_ASSERT_TRUE(TUsart::Spy::isInited());
			TEST_ASSERT_EQUAL_INT32(115200, TUsart::Spy::getBaudrate());
			TEST_ASSERT_EQUAL_INT(TUsart::Flags::DataBits5, TUsart::Spy::getDataBits());
			TEST_ASSERT_EQUAL_INT(TUsart::Flags::HalfStopBit, TUsart::Spy::getStopBits());
			TEST_ASSERT_EQUAL_INT(TUsart::Flags::OddParity, TUsart::Spy::getParity());
			TEST_ASSERT_EQUAL_INT(TUsart::Flags::RxEnable, TUsart::Spy::getMode());
		}

		static void Uart_Error() {
			TEST_ASSERT_EQUAL_INT(TUsart::Flags::NoError, TUsart::getError());
			TUsart::Spy::setError(TUsart::Flags::FramingError);
			TEST_ASSERT_EQUAL_INT(TUsart::Flags::FramingError, TUsart::getError());
			TEST_ASSERT_EQUAL_INT(TUsart::Flags::NoError, TUsart::getError());
		}




		static void Uart_SendData() {
			uint8_t data[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

			for (uint16_t i = 0; i < sizeof(data); i++) {
				TEST_ASSERT_TRUE(TUsart::writeByte(data[i]));
				TUsart::Spy::IntTxEmptyInterrupt(); //call tx interrupt after every byte send
			}

			UART_CHECK_SENDED(data, sizeof(data));
		}



		static void Uart_ReceiveData() {
			static uint8_t count_received = 0;
			static uint8_t byte_received = 0;
			TUsart::setOnReceive([](const uint8_t& value) {
				count_received++;
				byte_received = value;
			});
			UART_RECEIVE_BYTE(1);
			TEST_ASSERT_TRUE(count_received == 1);
			TEST_ASSERT_TRUE(byte_received == 1);
		}



		static void run() {
			RUN_TEST_CASE_CLASS(CphTest_AsyncUsart, Uart_Init_Default);
			RUN_TEST_CASE_CLASS(CphTest_AsyncUsart, Uart_Init);
			RUN_TEST_CASE_CLASS(CphTest_AsyncUsart, Uart_Error);
			RUN_TEST_CASE_CLASS(CphTest_AsyncUsart, Uart_SendData);
			RUN_TEST_CASE_CLASS(CphTest_AsyncUsart, Uart_ReceiveData);
		}
};


#undef UART_RECEIVE_BYTE
#undef UART_RECEIVE_ARRAY
#undef UART_CHECK_SENDED