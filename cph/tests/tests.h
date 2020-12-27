#pragma once
#include <unity/unity_fixture.h>
#include <cph/tests/test_timer.h>
#include <cph/tests/test_usart_async.h>
#include <cph/tests/fsm/test_fsm.h>
#include <cph/tests/fsm/test_hsm.h>
#include <cph/tests/test_ring_buffer.h>
#include <cph/tests/test_async.h>
//#include <cph/tests/wake/test_wake.h> TODO
inline void runCphTests() {
	RUN_TEST_GROUP_CLASS(CphTest_AsyncUsart);
	RUN_TEST_GROUP_CLASS(CphTest_Timer);
	RUN_TEST_GROUP_CLASS(CphTest_FSM);
	RUN_TEST_GROUP_CLASS(CphTest_HSM);
	RUN_TEST_GROUP_CLASS(CphTest_RingBuffer);
	RUN_TEST_GROUP_CLASS(CphTest_Async);
	RUN_TEST_GROUP(modbus);
	RUN_TEST_GROUP(ModbusCallbacksTestGroup);
	//	RUN_TEST_GROUP_CLASS(CphTest_Wake<0>);
}