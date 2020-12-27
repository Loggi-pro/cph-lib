#pragma once
#include <void/result.h>
#include <void/type_traits.h>
#include <void/functional.h>
namespace cph {
	//synchronizing async function
	//usage auto te = synchronize(lambda);
	//lambda must return ResultState, ex: [&]{return sensor.getTemperature()}

	using ResultState = vd::ResultState;
	template <typename T>
	using Result = vd::Result<T>;


	template <typename UnusedType, typename UnusedType2 = UnusedType>
	class Task;

	//template <typename ...Args>
	//class Task;

	template <typename ReturnType, typename ... ArgumentTypes>
	class Task <ReturnType(ArgumentTypes ...)> {
			cph::Result<ReturnType> _task;
		public:
			typedef cph::Result<ReturnType>(*FunctionT)(ArgumentTypes ...);
			Task() : _task(), mInvoker() {}
			//template <typename FunctionT>
			Task(FunctionT f) : _task(), mInvoker(f) {}
			//template <typename FunctionType, typename ClassType>
			//functionFunc(FunctionType ClassType::* f):mInvoker(f) {}

			cph::Result<ReturnType> operator ()(ArgumentTypes ... args) {
				if (_task.isComplete() == false) {
					_task = mInvoker.invoke(args ...);
				}

				if (_task.isComplete()) {
				}

				return _task;
			}
			bool isComplete() {
				return _task.isComplete();
			}
			bool isSuccess() {
				return _task.isSuccess();
			}
			bool isFail() {
				return !_task.isFail();
			}

			Task& run(ArgumentTypes ... args) {
				operator()(args...);
				return *this;
			}


			Task& waitFor(ArgumentTypes ...args) {
				while (!_task.isComplete()) {
					operator()(args...);
				}

				return *this;
			}
			ReturnType result() {
				//assert IsComplete()
				return _task.value();
			}




		private:
			class free_function_holder {
				private:
					FunctionT mFunction;
				public:
					free_function_holder(FunctionT func) : mFunction(func) {}

					cph::Result<ReturnType> invoke(ArgumentTypes ... args) {
						return mFunction(args ...);
					}

			};

			free_function_holder mInvoker;
	};


	template <typename ReturnType, typename ... ArgumentTypes, class TObj>
	class Task <ReturnType(ArgumentTypes ...), TObj> {
			cph::Result<ReturnType> _task;
		public:
			typedef cph::Result<ReturnType>(TObj::*FunctionT)(ArgumentTypes ...);
			Task() : _task(), mInvoker() {}
			//template <typename FunctionT>
			Task(TObj* obj, FunctionT f) : _task(), mInvoker(obj, f) {}

			cph::Result<ReturnType> operator ()(ArgumentTypes ... args) {
				if (_task.isComplete() == false) {
					_task = mInvoker.invoke(args ...);
				}

				return _task;
			}
			bool isComplete() {
				return _task.isComplete();
			}
			bool isSuccess() {
				return _task.isSuccess();
			}
			bool isFail() {
				return !_task.isFail();
			}

			Task& run(ArgumentTypes ... args) {
				operator()(args...);
				return *this;
			}


			Task& waitFor(ArgumentTypes ...args) {
				while (!_task.isComplete()) {
					operator()(args...);
				}

				return *this;
			}
			ReturnType result() {
				//assert IsComplete()
				return _task.value();
			}



			Task <ReturnType(ArgumentTypes ...), TObj>& bindTo(TObj* obj) {
				mInvoker.set(obj);
				return *this;
			}
		private:
			class object_function_holder {
				private:
					TObj* mObj;
					FunctionT mFunction;
				public:
					object_function_holder(TObj* obj, FunctionT func) : mObj(obj),
						mFunction(func) {}


					cph::Result<ReturnType> invoke(ArgumentTypes ... args) {
						return (*mObj.*mFunction)(args ...);
					}
					void set(TObj* obj) {
						mObj = obj;
					}

			};

			object_function_holder mInvoker;
	};




	//Deduction guides
	template <typename ReturnType, typename ... ArgumentTypes>
	Task(cph::Result<ReturnType>(*f)(ArgumentTypes...))->Task<ReturnType(ArgumentTypes...)>;

	template <typename ReturnType, typename ... ArgumentTypes, typename TObj>
	Task(TObj* obj, cph::Result<ReturnType>(*f)(
	         ArgumentTypes...))->Task<ReturnType(ArgumentTypes...), TObj>;

	//Functions

	template <typename F>
	auto synchronize(F f) -> decltype(f()) {
		using ReturnType = decltype(f());
		ReturnType res = f();

		do {
			res = f();
		} while (!res.isComplete());

		return res;
	}


	template <typename Task1, typename ... Tasks>
	static bool isAllComplete(Task1& t1, Tasks& ... t) {
		return t1.isComplete() && (... && t.isComplete());
	}

	//allow to wait all tasks
	template <typename Task1, typename ... Tasks>
	static void waitForAllTasks(Task1& t1, Tasks& ... t) {
		while (!isAllComplete(t1, t...)) {
			t1.run(), (..., t.run());
		}
	}


	//allow to bind any lambdas to task
	template <typename T, typename... Args>

	typename vd::enable_if < !vd::is_class<T>::value,
	         cph::Task<typename vd::func_parser<T>::ResultType::type(void) >>::type
	makeTask(T f, Args... args) {
		typedef typename vd::func_parser<T>::ResultType::type ResType; //type under Result
		return Task<ResType>([f, args...]() { return f(args...); });
	}


	template <typename O, typename T, typename... Args>
	typename vd::enable_if <
	vd::is_class <O>::value,
	cph::Task<typename vd::func_parser<T>::ResultType::type(void) >>::type
	makeTask(O& obj, T f, Args... args) {
		typedef typename vd::func_parser<T>::ResultType Res;
		auto func = vd::Private::fnptr<Res()>([&obj, f, args...]() mutable {
			return (obj.*f)(args...);
		});
		return cph::Task(func);
	}

}