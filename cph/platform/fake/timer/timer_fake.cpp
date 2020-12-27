#include "timer_fake.h"
#include <cph/timer/timer_types.h>
namespace cph::Private {
	bool TFakeTimer::_isInited = false;
	uint32_t TFakeTimer::_ocrValue = 0;
	TimerPrescalar TFakeTimer::_currentPrescalar = cph::TimerPrescalar::Stopped;
	bool TFakeTimer::_isEnabled = false;
	volatile TFakeTimer::timerTickCallback_t TFakeTimer::_cbOCRA_COMP = nullptr;

}
