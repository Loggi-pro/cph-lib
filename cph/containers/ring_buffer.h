#pragma once
#include <void/array.h>
#include <void/assert.h>

namespace cph {

	template  < typename T, size_t MAX_SIZE >
	class RingBuffer  {
			using Self  = RingBuffer < T, MAX_SIZE >;
			vd::Array < T, MAX_SIZE + 1 >  _container;
		public:
			//size of array is more on 1 element
			using counter_t = typename decltype(_container)::index_t;
			using data_t  = T;
		private:
			counter_t _bufferReadCounter = 0;
			counter_t _bufferWriteCounter = 0;

			inline void stepRead() {
				_bufferReadCounter = nextCounter(_bufferReadCounter);
			}
			[[nodiscard]] inline counter_t nextCounter(const counter_t counter) {
				counter_t tmp = (counter_t)(counter - 1);

				if (tmp == static_cast<counter_t>(-1)) {
					tmp = MAX_SIZE;
				}

				return static_cast<counter_t>(tmp);
			}
			[[nodiscard]] inline counter_t prevCounter(const counter_t counter) {
				counter_t tmp = (counter_t)(counter + 1);

				if (tmp == static_cast<counter_t>(MAX_SIZE + 1)) {
					tmp = 0;
				}

				return static_cast<counter_t>(tmp);
			}
		public:
			RingBuffer() = default;



			bool push(const data_t& data) {
				_container[_bufferWriteCounter] = data;
				counter_t tmp = nextCounter(_bufferWriteCounter);

				if (tmp == _bufferReadCounter) {
					//assert(false);//buffer overflow
					return false;
				} else {
					_bufferWriteCounter = tmp;
					return true;
				}
			}
			[[nodiscard]] data_t pop() {
				data_t tmp  = _container[_bufferReadCounter];
				stepRead();
				return tmp;
			}

			[[nodiscard]] T& back() {
				assert(!this->empty());
				return _container[prevCounter(_bufferWriteCounter)];
			}

			[[nodiscard]] T& front() {
				assert(!this->empty());
				return _container[_bufferReadCounter];
			}

			[[nodiscard]] counter_t size()const  {
				//reverse order
				uint8_t tmpWrite  = _bufferWriteCounter;
				uint8_t tmpRead  = _bufferReadCounter;

				if  (tmpWrite  > tmpRead) {
					counter_t distance  = (counter_t)(tmpWrite  - tmpRead);
					return (counter_t)(MAX_SIZE  + 1 - distance);
				} else  {
					return (counter_t)(tmpRead  - tmpWrite);
				}
			}
			[[nodiscard]] bool empty()const  {
				return _bufferReadCounter  == _bufferWriteCounter;
			}
			void clear() {
				_bufferWriteCounter  = 0;
				_bufferReadCounter  = 0;
			}
	};

	template  <typename T>
	class StreamAdapter  {
			using data_t  = typename T::data_t;
			T& _buf;
		public:
			StreamAdapter(T& buf) : _buf(buf) {}
			friend StreamAdapter& operator  << (StreamAdapter<T>& self,  const data_t& val) {
				self._buf.push(val);
				return self;
			}
			friend StreamAdapter& operator>>(StreamAdapter<T>& self,  data_t& res) {
				res  = self._buf.pop();
				return self;
			}
			bool empty()const  {
				return _buf.empty();
			}
	};


}
