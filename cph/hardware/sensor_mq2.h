#pragma once
#include <cph/timer.h>
#include <cph/gpio.h>
namespace cph {
	template <typename pinData, typename powerPin = cph::io::NullPin>
	class TSensor_MQ2 {
		private:
			static bool inline _isReady = false;
			static bool inline _isInited = false;
			static inline cph::timer_t _timer;
		public:
			static bool isReady() {
				if (_isReady) { return true; }

				if (!_isInited) { return false; }

				if (_timer.isElapsed(1_s)) {
					_isReady = true;
				}

				return _isReady;
			}

			static bool checkGas() {
				return !pinData::IsSet();
			}

			static void init() {
				powerPin::SetConfiguration(powerPin::Port::Out);
				pinData::SetConfiguration(pinData::Port::In);
				pinData::SetPullUp(pinData::PullMode::PullUp);
				powerPin::Set();
				_timer.start();
				_isReady = false;
				_isInited = true;
			}

			static void deinit() {
				powerPin::Clear();
				_isReady = false;
				_isInited = false;
			}

			static bool isInited() {
				return _isInited;
			}

	};

}