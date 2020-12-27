#include <void/delay.h>
#include <cph/system_counter.h>
#include "gui.h"

//TODO i2c not implemented
int main() {
	cph::TSystemCounter::initPrecision<cph::TimerPrecision::Resolution_10us, cph::TTimerNumber::TIMER_0>();
	Gui gui;

	while (!gui.init()) { continue; }

	gui.setScreen(&Gui::_initForm);

	while (!gui.update()) { continue; }

	vd::delay_ms<2000>();
	gui.setScreen(&Gui::_mainForm);
	gui.update();
	cph::timer_t timer;

	while (true) {
		gui.poll();

		if (timer.isElapsed(1000_ms)) {
			if (FCurrentSpeed > 0) {
				FCurrentSpeed--;
			} else {
				FCurrentSpeed += 500;
			}
		}
	}
}
