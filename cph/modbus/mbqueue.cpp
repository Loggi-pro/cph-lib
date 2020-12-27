#include "mbqueue.h"


void ModbusEventQueue::init() {
	_isEventInQueue = false;
}
bool ModbusEventQueue::postEvent(MBEventType eEvent) {
	_isEventInQueue = true;
	_queuedEvent = eEvent;
	return true;
}
bool ModbusEventQueue::getEvent(MBEventType* eEvent) {
	bool isEventHappened = false;

	if (_isEventInQueue) {
		*eEvent = _queuedEvent;
		_isEventInQueue = false;
		isEventHappened = true;
	}

	return isEventHappened;
}