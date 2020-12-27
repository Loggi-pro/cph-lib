#include <void/chrono.h>
#include <void/interrupt.h>
#include <cph/timer.h>
#include "hardware.h"
ostream cout;

int main() {
	pinLed::Port::Enable();
	pinLed::SetConfiguration(pinLed::Port::Out);
	cph::SystemCounter::initPrecision<TIMER_CONFIG>();
	vd::EnableInterrupts();
	cph::timer_t t;
	int i = 0;
	cout << "start" << cph::endl;

	while (true) {
		if (t.isElapsed(1000_ms)) {
			cout << i++ << cph::endl;
			pinLed::Toggle();
		}
	}

	return 0;
}