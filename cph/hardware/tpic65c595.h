#pragma once
#include <cph/protocols/latch.h>
#include <cph/gpio.h>
namespace cph {

	template<class ClockPin, class DataPin, class LatchPin, class EnablePin = cph::io::NullPin, class T = uint8_t>
	class TPic65c595: public cph::ThreePinLatch<ClockPin, DataPin, LatchPin, 0, T> {
			typedef cph::ThreePinLatch<ClockPin, DataPin, LatchPin, 0, T> TBase;
		public:
			static void Init() {
				TBase::Init();
				EnablePin::SetConfiguration(EnablePin::Port::Out);
				Disable();
			}

			static void Enable() {
				if (EnablePin::Inverted) {
					EnablePin::Set();
				} else {
					EnablePin::Clear(); //Active level = low
				}
			}

			static void Disable() {
				if (EnablePin::Inverted) {
					EnablePin::Clear();
				} else {
					EnablePin::Set(); //Active level = low
				}
			}
	};

	template
	<
	    typename T1 = cph::io::NullType,  typename ...Args >
	class TPic65c595Reader: public
		cph::io::PinList<T1, Args...> {
			typedef cph::io::PinList<T1, Args...> TBase;
		public:
			static void Init() {
				TBase::SetConfiguration(TBase::In);
				TBase::SetPullUp(TBase::PullUp);
			}


	};

}