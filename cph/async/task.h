#pragma once
#include "async_types.h"
namespace cph {
	template<typename T = void>
	class TResultAsync {
		private:
			TAsyncState _isComplete;
			T _value;
		public:
			typedef T type;
			TResultAsync(T aResValue) : _isComplete(TAsyncState::S_DONE),
				_value(aResValue) {}
			TResultAsync(TAsyncState state = TAsyncState::NO_RESULT) : _isComplete(state),
				_value() {}
			template <class U>
			TResultAsync(const TResultAsync<U>& other, T value): _isComplete(other.state()), _value(value) {
			}

			bool isComplete() const {
				return _isComplete != TAsyncState::NO_RESULT;
			}
			bool isSuccess() const {
				return _isComplete == TAsyncState::S_DONE;
			}
			bool isFail() const {
				return (isComplete()) && (!isSuccess());
			}
			TAsyncState state() const {
				return _isComplete;
			}
			const T& value()const  {
				return _value;
			}
			explicit operator bool()const {
				return isComplete();
			}
	};

	template<>
	class TResultAsync<void> {
		private:
			TAsyncState _isComplete;
		public:
			TResultAsync(TAsyncState state = TAsyncState::NO_RESULT) : _isComplete(
				    state) {}
			TResultAsync(bool result) : _isComplete(result == true ? TAsyncState::S_DONE :
				                                        TAsyncState::E_ERR)  {}
			bool isComplete()const {
				return _isComplete != TAsyncState::NO_RESULT;
			}
			bool isSuccess()const {
				return _isComplete == TAsyncState::S_DONE;
			}
			bool isFail()const {
				return (isComplete()) && (!isSuccess());
			}
			explicit operator bool()const {
				return isComplete();
			}
			TAsyncState state() const {
				return _isComplete;
			}
	};



	template <typename UnusedType, typename UnusedType2 = UnusedType>
	class TTaskAsync;



	template <typename ReturnType, typename ... ArgumentTypes>
	class TTaskAsync <ReturnType(ArgumentTypes ...)> {
			TResultAsync<ReturnType> _task;
		public:
			typedef TResultAsync<ReturnType>(*FunctionT)(ArgumentTypes ...);
			TTaskAsync() : _task(), mInvoker() {}
			//template <typename FunctionT>
			TTaskAsync(FunctionT f) : _task(), mInvoker(f) {}
			//template <typename FunctionType, typename ClassType>
			//functionFunc(FunctionType ClassType::* f):mInvoker(f) {}

			TResultAsync<ReturnType> operator ()(ArgumentTypes ... args) {
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

			TTaskAsync& run(ArgumentTypes ... args) {
				operator()(args...);
				return *this;
			}


			TTaskAsync& waitFor(ArgumentTypes ...args) {
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

					TResultAsync<ReturnType> invoke(ArgumentTypes ... args) {
						return mFunction(args ...);
					}

			};

			free_function_holder mInvoker;
	};


	template <typename ReturnType, typename ... ArgumentTypes, class TObj>
	class TTaskAsync <ReturnType(ArgumentTypes ...), TObj> {
			TResultAsync<ReturnType> _task;
		public:
			typedef TResultAsync<ReturnType>(TObj::*FunctionT)(ArgumentTypes ...);
			TTaskAsync() : _task(), mInvoker() {}
			//template <typename FunctionT>
			TTaskAsync(TObj* obj, FunctionT f) : _task(), mInvoker(obj, f) {}

			TResultAsync<ReturnType> operator ()(ArgumentTypes ... args) {
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

			TTaskAsync& run(ArgumentTypes ... args) {
				operator()(args...);
				return *this;
			}


			TTaskAsync& waitFor(ArgumentTypes ...args) {
				while (!_task.isComplete()) {
					operator()(args...);
				}

				return *this;
			}
			ReturnType result() {
				//assert IsComplete()
				return _task.value();
			}



			TTaskAsync <ReturnType(ArgumentTypes ...), TObj>& bindTo(TObj* obj) {
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


					TResultAsync<ReturnType> invoke(ArgumentTypes ... args) {
						return (*mObj.*mFunction)(args ...);
					}
					void set(TObj* obj) {
						mObj = obj;
					}

			};

			object_function_holder mInvoker;
	};
}
