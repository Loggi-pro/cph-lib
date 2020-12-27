#pragma once
#include <unity/unity_fixture_classes.h>
#include <cph/containers/ring_buffer.h>
#include <void/type_traits.h>
#include <void/static_assert.h>

VOID_STATIC_ASSERT(vd::is_same_v<cph::RingBuffer<uint8_t, 254>::counter_t, uint8_t>);
VOID_STATIC_ASSERT(vd::is_same_v<cph::RingBuffer<uint8_t, 255>::counter_t, uint16_t>);

class CphTest_RingBuffer : UnityTestClass<CphTest_RingBuffer> {
		static const uint8_t SIZE = 32;
		struct Entry {
			u16 data1;
			u16 data2;
			Entry():data1(0),data2(0){}
		};
		inline static cph::RingBuffer <Entry, SIZE> _rb = cph::RingBuffer <Entry, SIZE> {};
		static void fillBuffer() {
			Entry e;

			for (uint8_t i = 0; i < SIZE; ++i) {
				e.data2 = i;
				_rb.push(e);
			}
		}
	public:

		static void setup() {
			_rb.clear();
		}
		static void tearDown() {}
		static void  CheckBuffer_ShouldEmpty() {
			TEST_ASSERT_EQUAL(true, _rb.empty());
			TEST_ASSERT_EQUAL(0, _rb.size());
		}

		static void Check_Push_NotEmpty() {
			Entry e;
			TEST_ASSERT_EQUAL(true, _rb.push(e));
			TEST_ASSERT_EQUAL(false, _rb.empty());
		}


		static void Check_PushToFull() {
			Entry e;

			for (u08 i = 0; i < SIZE; i++) {
				e.data2 = i;
				TEST_ASSERT_EQUAL(true, _rb.push(e));
				TEST_ASSERT_EQUAL(i + 1, _rb.size());
			}

			TEST_ASSERT_EQUAL(SIZE, _rb.size());
		}





		static void Check_PopToEmpty() {
			fillBuffer();

			for (u08 i = 0; i < SIZE; i++) {
				const Entry entry = _rb.pop();
				TEST_ASSERT_EQUAL(i, entry.data2);
				TEST_ASSERT_EQUAL(SIZE - 1 - i, _rb.size());
			}

			TEST_ASSERT_EQUAL(0, _rb.size());
		}

		static void Check_OnBufferOverride_ShouldNotEraseData() {
			fillBuffer();
			Entry e = _rb.pop();
			//override buffer 8 times
			fillBuffer();

			for (int i = 0; i < SIZE; i++) {
				e = _rb.pop();
				TEST_ASSERT_EQUAL((i + 1) % SIZE, e.data2);
			}

			TEST_ASSERT_EQUAL(0, _rb.size());
		}

		static void Check_Back() {
			Entry e;

			for (u08 i = 0; i < SIZE; i++) {
				e.data2 = i;
				_rb.push(e);
				TEST_ASSERT_EQUAL(e.data2, _rb.back().data2);
			}
		}
		static void Check_Front() {
			Entry e;
			fillBuffer();

			for (u08 i = 0; i < SIZE; i++) {
				e = _rb.front();
				TEST_ASSERT_EQUAL(i, e.data2);
				e = _rb.pop();
			}
		}


		static void run() {
			RUN_TEST_CASE_CLASS(CphTest_RingBuffer, CheckBuffer_ShouldEmpty);
			RUN_TEST_CASE_CLASS(CphTest_RingBuffer, Check_Push_NotEmpty);
			RUN_TEST_CASE_CLASS(CphTest_RingBuffer, Check_PushToFull);
			RUN_TEST_CASE_CLASS(CphTest_RingBuffer, Check_PopToEmpty);
			RUN_TEST_CASE_CLASS(CphTest_RingBuffer, Check_OnBufferOverride_ShouldNotEraseData);
			RUN_TEST_CASE_CLASS(CphTest_RingBuffer, Check_Back);
			RUN_TEST_CASE_CLASS(CphTest_RingBuffer, Check_Front);
		}
};