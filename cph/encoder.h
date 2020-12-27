#pragma once
#include <cph/gpio.h>
#include <cph/exti.h>
//https://m.habr.com/ru/post/340448/
namespace cph {
	namespace Private {

		struct _incrementTable {
			inline static constexpr int _increment[16] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};
		};
	}
	template <typename pinA, typename pinB>
	class encoder_t {
			typedef cph::io::PinList<pinA, pinB> group;
			static inline volatile long _angle = 0;
			static inline volatile char _ABprev = 0;
			static uint8_t _readState() {
				return group::PinRead();
			}
		public:

			static void IntHandler() {
				uint8_t AB = _readState();
				_angle += Private::_incrementTable::_increment[AB + _ABprev * 4];
				_ABprev = AB;
			}

			static void init() {
				pinA::Port::Enable();
				pinB::Port::Enable();
				group::SetConfiguration(group::In);
				cph::exti::attachInterrupt<pinA, pinB>(IntHandler, cph::EXTI_BOTH);
				_ABprev = _readState();
			}
			uint16_t angle() {
				return _angle;
			}

	};

}
