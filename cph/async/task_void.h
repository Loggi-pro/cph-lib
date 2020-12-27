#pragma once
#include "async_types.h"
#include "task.h"

namespace cph {


using TVoidResultAsync = TResultAsync<void>;



template <typename UnusedType, typename UnusedType2 = UnusedType>
class TVoidTaskAsync;

template <typename ... ArgumentTypes>
class TVoidTaskAsync <void(ArgumentTypes ...)> {
		TVoidResultAsync _task;
	public:
		typedef TVoidResultAsync(*FunctionT)(ArgumentTypes ...);
		TVoidTaskAsync() : _task(), mInvoker() {}
		//template <typename FunctionT>
		TVoidTaskAsync(FunctionT f) : _task(), mInvoker(f) {}
		//template <typename FunctionType, typename ClassType>
		//functionFunc(FunctionType ClassType::* f):mInvoker(f) {}

		TVoidResultAsync operator ()(ArgumentTypes ... args) {
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

		TVoidTaskAsync& run(ArgumentTypes ... args) {
			operator()(args...);
			return *this;
		}


		TVoidTaskAsync& waitFor(ArgumentTypes ...args) {
			while (!_task.isComplete()) {
				operator()(args...);
			}

			return *this;
		}
		/*
				void result() {
					//assert IsComplete()
					return _task.value();
				}
		*/




	private:
		class free_function_holder {
			private:
				FunctionT mFunction;
			public:
				free_function_holder(FunctionT func) : mFunction(func) {}

				TVoidResultAsync invoke(ArgumentTypes ... args) {
					return mFunction(args ...);
				}

		};

		free_function_holder mInvoker;
};

template <typename ... ArgumentTypes, class TObj>
class TVoidTaskAsync <void(ArgumentTypes ...), TObj> {
		TVoidResultAsync _task;
	public:
		typedef TVoidResultAsync(TObj::*FunctionT)(ArgumentTypes ...);
		TVoidTaskAsync() : _task(), mInvoker() {}
		//template <typename FunctionT>
		TVoidTaskAsync(TObj* obj, FunctionT f) : _task(), mInvoker(obj, f) {}

		TVoidResultAsync operator ()(ArgumentTypes ... args) {
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

		TVoidTaskAsync& run(ArgumentTypes ... args) {
			operator()(args...);
			return *this;
		}


		TVoidTaskAsync& waitFor(ArgumentTypes ...args) {
			while (!_task.isComplete()) {
				operator()(args...);
			}

			return *this;
		}
		/*
		void result() {
		//assert IsComplete()
		return _task.value();
		}*/



		TVoidTaskAsync <void(ArgumentTypes ...), TObj>& bindTo(TObj* obj) {
			mInvoker.set(obj);
			return *this;
		}
	private:
		class object_function_holder {
			private:
				FunctionT mFunction;
				TObj* mObj;
			public:
				object_function_holder(TObj* obj, FunctionT func) : mObj(obj),
					mFunction(func) {}


				TVoidResultAsync invoke(ArgumentTypes ... args) {
					return (*mObj.*mFunction)(args ...);
				}
				void set(TObj* obj) {
					mObj = obj;
				}

		};

		object_function_holder mInvoker;
};

}
