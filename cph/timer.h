#pragma once
#include <void/chrono.h>
#include <cph/system_counter.h>
#include <void/atomic.h>
namespace cph {

	struct timer_t {
		private:
			uint32_t _usValue;
			/*vd::chrono::micro_t  _get() {
				TTimer::counter_t lCurrentTime = TTimer::counter();

				if (_value <= lCurrentTime) {
					return (lCurrentTime - _value);
				}

				//было переполнение системного таймера
				TTimer::counter_t _value = ~(l_current_time) | (l_current_time);
				return  (max_value - _value) + 1 + //часть до переполнения
				    l_current_time;// часть до переполнения

			}*/
		public:
			typedef typename SystemCounter::counter_t counter_t;
			timer_t(): _usValue(SystemCounter::ticks_us()) {
			}

			timer_t(const timer_t& other) : _usValue(other._usValue) {}



			void start() {
				_usValue = SystemCounter::ticks_us();
				//::vd::Atomic::Set(&_usValue, TSystemCounter::ticks_us());
			}
			uint32_t elapsed_us()const {
				//value =200
				//counter= 50;
				//!IMPORTANT
				//!FIRST - fetch _usValue (IT CAN BE CHANGE FROM INTERRUPT)
				//!SECOND - fetch now ticks (IT  MUST BE GREATER IF NO OVERFLOW)
				//counter_t usValue = _usValue; //::vd::Atomic::Fetch(&_usValue);
				counter_t now = SystemCounter::ticks_us();//TSystemCounter::ticks_us();
				counter_t usDiff = now - _usValue; //учитывается переполнение
				return usDiff;
			}
			template <typename Period>
			bool isElapsed(const ::vd::chrono::any_t<Period>& duration) {
				//ATOMIC_BLOCK(){
				uint32_t usDuration = ::vd::chrono::micro_t(duration).count();
				uint32_t elapsed = elapsed_us();
				bool result = elapsed >= usDuration;

				if (result) {
					start();
				}

				return result;
				//}
			}
			template <typename Period>
			counter_t remains(const ::vd::chrono::any_t<Period>& duration) {
				uint32_t usDuration = ::vd::chrono::micro_t(duration).count();
				uint32_t elapsed = elapsed_us();

				if (elapsed < usDuration) {
					return usDuration - elapsed;
				} else {
					return 0;
				}
			}
			#ifdef _VOID_NATIVE_CPP17_CONSTEXPR_SUPPORT
			//For literal operator time support (like _ms,_h etc.)
			template <typename T>
			constexpr bool isElapsed(const T literal) {
				uint32_t elapsed = elapsed_us();
				bool result = elapsed >= T::to_micro::static_value;

				if (result) {
					start();
				}

				return result;
			}
			#endif
	};

}
