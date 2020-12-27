#pragma once
#include <unity/unity_fixture_classes.h>
#include <void/result.h>
#include <cph/async.h>

class TestObject {
		int a;
	public:
		TestObject(int b) : a(b) {}
		cph::Result<int> method_wait(int cycles) {
			static int cnt = 0;
			cnt++;

			if (cnt < cycles) {
				return cph::Result<int>();
			}

			return cph::Result <int>(a);
		}


		cph::Result<void> method_void() {
			return cph::Result<void>(cph::ResultState::S_DONE);
		}
};
class TTestFsm {
	private:
		int i;


	public:
		TTestFsm() : i(0) {
		}
		cph::Result<int> internalFunc(uint8_t val) {
			if (i < 5) {
				i++;
				return cph::Result<int>(cph::ResultState::NO_RESULT);
			} else {
				return cph::Result<int>(val);
			}
		}
};





class CphTest_Async : UnityTestClass<CphTest_Async> {
		static cph::Result<void> _feature_func_call_test() {
			return { cph::ResultState::S_DONE };
		}
		static cph::Result<void> _func_void2() {
			static int i = 1;
			i++;

			if (i < 2) {
				return {};
			}

			i = 1;
			return { cph::ResultState::S_DONE };
		}



	public:

		static void setup() {
		}
		static void tearDown() {}

		static void AsyncFeature_Func_OnCreate_ShouldBeFalse() {
			auto f = cph::Task(_feature_func_call_test);
			TEST_ASSERT_EQUAL(false, f.isComplete());
		}

		static void AsyncFeature_VoidFunc_VoidRet_ShouldWork() {
			auto f = cph::Task<void(void)>(_func_void2);
			TEST_ASSERT_EQUAL(false, f.isComplete());
			f.run();
			TEST_ASSERT_EQUAL(true, f.isComplete());
		}
		static void func(void) {
		}

		static void AsyncFeature_VoidFunc_VoidTask_ShouldWork() {
			auto f = cph::Task(_func_void2);
			TEST_ASSERT_EQUAL(false, f.isComplete());
			f.run();
			TEST_ASSERT_EQUAL(true, f.isComplete());
		}

		static void AsyncCore_Func_NoArgWithRet_ShoudlWork() {
			auto ret_value_func = []()  {
				static int i = 0;
				i++;

				if (i < 2) { return cph::Result<int>(); }

				i = 1;
				return cph::Result<int>(1);
			};
			auto f = cph::Task<int(void)>(ret_value_func);
			TEST_ASSERT_EQUAL(false, f.isComplete());
			f.run();
			TEST_ASSERT_EQUAL(true, f.isComplete());
			TEST_ASSERT_EQUAL(f.result(), 1);
		}

		static void AsyncFeature_Func_OnMultipleCall_ShouldBeCallOnce() {
			auto feature_func_callableOnce = []()->cph::Result<int> {
				static int a = 0;
				a++;
				return a;
			};
			auto f = cph::Task<int(void)>(feature_func_callableOnce);
			f.run();
			TEST_ASSERT_EQUAL(true, f.isComplete());
			TEST_ASSERT_EQUAL(1, f.result());
			f.run(); //TestObject that this proc only once
			TEST_ASSERT_EQUAL(true, f.isComplete());
			TEST_ASSERT_EQUAL(1, f.result() );
		}



		static void AsyncFeature_Func_ArgWithRet_ShoudlWork() {
			auto f = cph::Task<int(const int*, int)>([](const int* x, int y)->cph::Result<int> { return (*x) + y; });
			int x = 1;
			f.run(&x, 4);
			TEST_ASSERT_EQUAL(true, f.isComplete());
			TEST_ASSERT_EQUAL(5, f.result());
		}

		static void AsyncCore_Func_OnLoop_ShouldBeCalledNessesaryTimes() {
			auto f = cph::Task<int(int)>([](int cycles) {
				static int cnt = 0;
				cnt++;

				if (cnt < cycles) {
					return cph::Result<int>();
				}

				return cph::Result <int>(2);
			});
			int counter = 0;
			const int CYCLES = 5;

			while (!f.isSuccess()) {
				TEST_ASSERT_EQUAL(false, f.isComplete());
				f.run(CYCLES);
				counter++;
			}

			TEST_ASSERT_EQUAL(true, f.isComplete());
			TEST_ASSERT_EQUAL(CYCLES, counter);
			TEST_ASSERT_EQUAL(2, f.result());
		}

		static void AsyncFeature_Func_OnLoop_WaitForShouldWork() {
			auto f = cph::Task<int(int)>([](int cycles) mutable {
				static int cnt = 0;
				cnt++;

				if (cnt < cycles) {
					return cph::Result<int>();
				}

				return cph::Result <int>(2);
			});
			const int CYCLES = 5;
			f.waitFor(CYCLES);
			TEST_ASSERT_EQUAL(true, f.isComplete());
			TEST_ASSERT_EQUAL(2, f.result());
		}


		static void AsyncCore_Method_ArgWithRet_ShoudlWork() {
			TestObject o1(2);
			auto f = cph::Task<int(int), TestObject>(&o1, &TestObject::method_wait);
			static int CYCLES = 5;
			f.waitFor(CYCLES);
			TEST_ASSERT_EQUAL(true, f.isComplete());
			TEST_ASSERT_EQUAL(2, f.result());
		}

		static void AsyncFeature_Method_OnBinding_ShouldBeCalledForBindedObject() {
			TestObject o1(2);
			TestObject o2(4);
			auto f = cph::Task<int(int), TestObject>(&o1, &TestObject::method_wait);
			static int CYCLES = 5;
			f.bindTo(&o2).waitFor(CYCLES);
			TEST_ASSERT_EQUAL(true, f.isComplete());
			TEST_ASSERT_EQUAL(4, f.result());
		}


		static void AsyncFeature_MakeTask() {
			TTestFsm fsm;
			auto task1 = cph::makeTask(fsm, &TTestFsm::internalFunc, (uint8_t)5);
			//��� ����������
			task1.waitFor();
			//... ����� �����
			TEST_ASSERT_EQUAL(true, task1.isComplete());
			TEST_ASSERT_EQUAL(5, task1.result());
		}

		inline static int counter_task1 = 0;
		inline static int counter_task2 = 0;
		inline static int counter_task3 = 0;
		static void Test_WaitForAllTasks() {
			TTestFsm fsm;
			auto t1 = cph::Task<void(void)>([]()->cph::Result<void> {
				//std::cout << "invoke task 1 " << std::endl;
				counter_task1++;

				if (counter_task1 > 0) {
					//std::cout << "done 1 " << std::endl;
					return { cph::ResultState::S_DONE };
				}

				return { cph::ResultState::NO_RESULT };
			});
			auto t2 = cph::Task<void(void)>([]()->cph::Result<void> {
				counter_task2++;
				//std::cout << "invoke task 2 " << std::endl;

				if (counter_task2 > 1) {
					//std::cout << "done 2 " << std::endl;
					return { cph::ResultState::S_DONE };
				}

				return { cph::ResultState::NO_RESULT };
			});
			auto t3 = cph::Task<void(void)>([]()->cph::Result<void> {
				counter_task3++;
				//std::cout << "invoke task 3 " << std::endl;

				if (counter_task3 > 2) {
					//std::cout << "done 3 " << std::endl;
					return { cph::ResultState::S_DONE };
				}

				return { cph::ResultState::NO_RESULT };
			});
			waitForAllTasks(t1, t2, t3);
			TEST_ASSERT_EQUAL(true, t1.isComplete());
			TEST_ASSERT_EQUAL(true, t2.isComplete());
			TEST_ASSERT_EQUAL(true, t3.isComplete());
			TEST_ASSERT_EQUAL(1, counter_task1); //called only once
			TEST_ASSERT_EQUAL(2, counter_task2); //called twice
			TEST_ASSERT_EQUAL(3, counter_task3); //called three times
		}
		static void run() {
			RUN_TEST_CASE_CLASS(CphTest_Async, AsyncFeature_Func_OnCreate_ShouldBeFalse);
			RUN_TEST_CASE_CLASS(CphTest_Async, AsyncFeature_VoidFunc_VoidRet_ShouldWork);
			RUN_TEST_CASE_CLASS(CphTest_Async, AsyncFeature_VoidFunc_VoidTask_ShouldWork);
			RUN_TEST_CASE_CLASS(CphTest_Async, AsyncFeature_Func_OnMultipleCall_ShouldBeCallOnce);
			RUN_TEST_CASE_CLASS(CphTest_Async, AsyncFeature_Func_ArgWithRet_ShoudlWork);
			RUN_TEST_CASE_CLASS(CphTest_Async, AsyncCore_Func_OnLoop_ShouldBeCalledNessesaryTimes);
			RUN_TEST_CASE_CLASS(CphTest_Async, AsyncFeature_Func_OnLoop_WaitForShouldWork);
			RUN_TEST_CASE_CLASS(CphTest_Async, AsyncCore_Method_ArgWithRet_ShoudlWork);
			RUN_TEST_CASE_CLASS(CphTest_Async, AsyncFeature_Method_OnBinding_ShouldBeCalledForBindedObject);
			RUN_TEST_CASE_CLASS(CphTest_Async, AsyncFeature_MakeTask);
			RUN_TEST_CASE_CLASS(CphTest_Async, Test_WaitForAllTasks);
		}
};



















/*
cph::Result<int> func_waitCycles(int cycles) {
	static int cnt = 0;
	cnt++;

	if (cnt < cycles) {
		return cph::Result<int>();
	}

	return cph::Result <int>(2);
}



template<typename T = void>
class TAsyncTask {
	private:
		Result<T> _res;
		Result<T>(*_ptr)();
		void _runOnce() {
			if (!_res.isComplete()) {
				_res = (*_ptr)();
			}
		}
	public:
		template <class Lambda>
		TAsyncTask(Lambda l) {
			_ptr = vd::Private::fnptr<Result<T>()>(l);
		}
		bool isComplete() {
			_runOnce();
			return _res.isComplete();
		}

		void wait() {
			while (!isComplete()) { continue; }
		}

		bool isSuccess() {
			_runOnce();
			return _res.isSuccess();
		}
		bool isFail() {
			_runOnce();
			return _res.isFail();
		}
		T& value() {
			return _res.value();
		}
};


class TTestFsm {
	private:
		int i;


	public:
		TTestFsm() : i(0) {
		}
		Result<int> internalFunc(uint8_t val) {
			switch (i) {
			case 0: i++; break;

			case 1: i++; break;

			case 2: i++; break;

			case 3: i++; break;

			case 4: i++; break;

			case 5://���� ������� �� ��� ����, �� ���������� �����
				return Result<int>(val);
			}

			//�� ���� ��������� ������� �������� ��� �� ���������
			return Result<int>(ResultState::NO_RESULT);
		}

		TAsyncTask<int> Fsm(int val) {
			TAsyncTask<int> obj([this, val]() mutable {return this->internalFunc(val); });
			return obj;
		}

};





static int i = 0;
cph::Result<int> internalFunc(uint8_t val) {
	switch (i) {
	case 0: i++; break;

	case 1: i++; break;

	case 2: i++; break;

	case 3: i++; break;

	case 4: i++; break;

	case 5://���� ������� �� ��� ����, �� ���������� �����
		return cph::Result<int>(val);
	}

	//�� ���� ��������� ������� �������� ��� �� ���������
	return cph::Result<int>(cph::ResultState::NO_RESULT);
}

Result<int> internalFunc2() {
	switch (i) {
	case 0: i++; break;

	case 1: i++; break;

	case 2: i++; break;

	case 3: i++; break;

	case 4: i++; break;

	case 5://���� ������� �� ��� ����, �� ���������� �����
		return cph::Result<int>(40);
	}

	//�� ���� ��������� ������� �������� ��� �� ���������
	return cph::Result<int>(cph::ResultState::NO_RESULT);
}


static void Test2() {
	TTestFsm fsm;
	//typedef typename vd::func_parser<Result<int>(*)>::ResultType a;
	vd::Task<int> task0 = makeTask(internalFunc2);
	vd::Task<int> task1 = makeTask(fsm, &TTestFsm::internalFunc, 5);
	vd::delegate_t<Result<int>()> task2 = vd::delegate::make(internalFunc2);
	vd::delegate_t<Result<int>()> task3 = vd::delegate::make(fsm, &TTestFsm::internalFunc, 5);
	auto f = vd::bind(internalFunc, 5);
	task2();

	//������ ���-�� ���
	if (task0.isComplete()) {
		//���� ������ �� ���������...
	}

	//��� ����������
	task0.wait();
	task1.wait();
	//... ����� �����
	TEST_ASSERT_EQUAL(40, task0.value());
	TEST_ASSERT_EQUAL(5, task1.value());
}

*/
