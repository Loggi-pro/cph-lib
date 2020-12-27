#include <void/interrupt.h>
#include <void/static_assert.h>
#include <cph/system_counter.h>
#include <cph/gpio.h>
#include <loggi/tasker.h>


typedef  cph::TTimerConfig<cph::TIMER_0, cph::TimerPrecision::Resolution_1ms> TIMER_CONFIG;
typedef cph::io::Pb5 pinLed;
typedef cph::io::Pb6 pinLed2;

using namespace cph::tasker;
VOID_ISR(CPH_INT_TIMER0_OCRA) {
	cph::Private::timer0::THwTimer0::IntOCRA_Handler();
}

void PeriodicTask();
void EventHandlerTask();
void TimeTask();
using periodicTask = cph::tasker::Task<PeriodicTask>;
using onEventHappenTask = Task<EventHandlerTask>;
using timeTask = Task<TimeTask, onEventHappenTask>;
typedef Tasker< periodicTask, timeTask, onEventHappenTask> TaskManager;

void PeriodicTask() {
	pinLed2::Toggle();
	TaskManager::getCurrentTask().suspendFor(1000_ms);
}

void EventHandlerTask() {
	static bool enabled = false;
	enabled = !enabled;

	if (enabled) {
		pinLed::Set();
		TaskManager::startTaskAfter<onEventHappenTask>(1000_ms);
	} else {
		pinLed::Clear();
		TaskManager::getCurrentTask().suspend();
	}
}

void TimeTask() {
	//TaskManager::getCurrentTask().notifyAll();
	TaskManager::getCurrentTask().suspendFor(3000_ms);
}



int main() {
	pinLed::Port::Enable();
	pinLed::SetConfiguration(pinLed::Port::Out);
	pinLed2::Port::Enable();
	pinLed2::SetConfiguration(pinLed2::Port::Out);
	cph::TSystemCounter::initPrecision<TIMER_CONFIG>();
	vd::EnableInterrupts();
	TaskManager::startTask<timeTask>();
	TaskManager::startTask<periodicTask>();

	while (true) {
		TaskManager::poll();
	}

	return 0;
}