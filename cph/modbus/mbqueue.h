#pragma once
/* ----------------------- Type definitions ---------------------------------*/



class ModbusEventQueue {
	public:
		enum MBEventType {
			EV_READY,                   /*!< Startup finished. */
			EV_FRAME_RECEIVED,          /*!< Frame received. */
			EV_EXECUTE,                 /*!< Execute function. */
			EV_FRAME_SENT               /*!< Frame sent. */
		};
	private:
		MBEventType _queuedEvent;
		bool _isEventInQueue;
	public:
		void init();
		bool postEvent(MBEventType eEvent);
		bool getEvent(MBEventType* eEvent);
};
