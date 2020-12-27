#pragma once
#include <void/dateutils.h>
namespace cph {
	class Calendar {
			static inline TDateTime _timeStruct  = TDateTime{};
			static inline bool _wasUpdated = false;
			static inline bool _wasInited = false;
			static inline s08 _timezone = 0;
		public:
			static void init(TDateTime timestruct, s08 timezone) {
				_timeStruct = timestruct;
				_timezone = timezone;
				_wasInited = true;
			}
			static bool isInited() {
				return _wasInited;
			}

			static const TDateTime& getTime() {
				return _timeStruct;
			}

			static TDateTime copyTime() {
				return _timeStruct;
			}

			static s08 getTimezone() {
				return _timezone;
			}
			static void setTimezone(s08 newTimezone) {
				_timezone = newTimezone;
				_wasUpdated = true;
			}

			static void setTime(const TDateTime& time) {
				_timeStruct = time;
				_wasUpdated = true;
			}
			static bool isUpdated() {
				bool temp = _wasUpdated;
				_wasUpdated = false;
				return temp;
			}
			static void poll(void) {
				if (!_wasInited) { return; }

				_timeStruct.IncDateTimeSeconds(1);
			}
	};
}

//TODO Create updater class for different sources
//#define CALENDAR_RENEW_TIME (1*MINUTE) //RENEW FROM SIM900\NTP
//#define CALENDAR_TASK_REPEAT_TIMER (10*SEC)
//#define CALENDAR_USE_SIM900
//#define CALENDAR_USE_NTP //defined in HAL_server
//#define CALENDAR_USE_TIMEZONE //defined in HAL_server

/* Basic Updater class
setTime();
			if (_currentTask == TASK_GETTIME) {
				state_GetRemoteTime = RTC_READ_REMOTE;
				ReleaseTask(TASK_GETTIME);
			}

class CalendarUpdater {
		typedef enum {
			TASK_IDLE, TASK_WAIT, TASK_GETTIME, TASK_SETTIME
		} TCalendar_Task;
		typedef enum {
			RTC_READ_REMOTE,
			RTC_READ_SIM900,
			RTC_READ_NTP,
			RTC_OK,
		} TCalendar_state;

		static inline bool _renewNow = true;

		static inline TCalendar_Task _currentTask = TASK_IDLE;
		static Ttimer timer_RTC;
		static Ttimer timer_ForTasks;

		u32 timer_rtc_waittime = CALENDAR_RENEW_TIME;

		static Ttimer timer_tick;
		static bool F_need_to_set_remote_time = false;
		bool _synchronized = false; //is received remote time from start of MCU?

		bool _setTask(TCalendar_Task task) {
			if (_currentTask != TASK_IDLE) { return false; }

			_currentTask = task;
			return true;
		}
		void _releaseTask(TCalendar_Task task) {
			if (_currentTask == task) {
				_currentTask = TASK_IDLE;
			}
		}
		void init () {
			timer_RTC.reset();
			timer_rtc_waittime = CALENDAR_RENEW_TIME;
			_renewNow = true;
			_synchronized = false;
			timer_tick.reset();
			F_need_to_set_remote_time = false;
		}
		void tick() {
			static ttimer_val remain_time = 0;
			ttimer_val elapsed_time = timer_tick.get() + remain_time;

			if (elapsed_time < 1 * SEC) { return; }

			timer_tick.reset();
			remain_time = elapsed_time - 1 * SEC;
		}
		void poll() {
			switch (_currentTask) {
			case TASK_IDLE:
				if (F_need_to_set_remote_time && timer_ForTasks.get() > CALENDAR_TASK_REPEAT_TIMER) {
					//������ ����� ����� ����� ���� ����� �� ������� ����������, ������� �� ���������� �� �����
					timer_ForTasks.reset();
					dprintf(PSTR("set task set remote time\n"));
					SetTask(TASK_SETTIME);
				} else {
					if ((timer_RTC.get() > timer_rtc_waittime || _renewNow)) {
						state_GetRemoteTime = RTC_READ_REMOTE;
						SetTask(TASK_GETTIME);
					}
				}

				break;

			case TASK_WAIT:
				//Waiting for something (For Use Calendar_EndUpdate or Calendar_RemoteUpdate);
				break;

			case TASK_GETTIME:
				if (GetRemoteTime(&_timeStruct)) {
					timer_RTC.reset();
					_renewNow = false;

					if (Calendar_Synchronized()) {
						timer_rtc_waittime = CALENDAR_RENEW_TIME;
					} else {
						timer_rtc_waittime = 5 * SEC;
					}

					ReleaseTask(TASK_GETTIME);
				}

				break;

			case TASK_SETTIME:
				if (SetRemoteTime(&_timeStruct)) {
					ReleaseTask(TASK_SETTIME);
					dprintf(PSTR("release task set remote time\n"));
				}

				break;
			}
		}
		//Dont get remote time
		bool beginUpdate() {
			return SetTask(TASK_WAIT);
		}
//Get remote time
		bool endUpdate() {
			ReleaseTask(TASK_WAIT);
		}
//Set remote time as local
		void remoteUpdate(void) {
			F_need_to_set_remote_time = true;
			SetTask(TASK_SETTIME);
		}

		bool synchronized() {
			return _synchronized;
		}

		void setRemote(const TDateTime* time ) {
			Calendar_SetTime(time);
			Calendar_RemoteUpdate();
		}
};
*/
/* Different updater class for SIM900/ntp

		static TCalendar_state state_GetRemoteTime = RTC_READ_REMOTE;

bool GetRemoteTime(TDateTime* timestruct) {
	bool result = false;
	TResult res = NO_RESULT;
	char str[17] ;

	switch (state_GetRemoteTime) {
	case RTC_READ_REMOTE:
		#ifdef CALENDAR_USE_SIM900
		state_GetRemoteTime = RTC_READ_SIM900;
		break;
		#endif
		#ifdef CALENDAR_USE_NTP
		state_GetRemoteTime = RTC_READ_NTP;
		break;
		#endif
		break;

	case RTC_READ_SIM900:
		#ifdef CALENDAR_USE_SIM900
		res = Sim900_ReadTime(timestruct);

		if ( res != NO_RESULT) {
			if (res == S_DONE) {
				Calendar_ToString(str);
				dprintf(PSTR("Get sim900 time=<%s>\n"), str);
			}

			state_GetRemoteTime = RTC_READ_NTP;
		}

		#else
		state = RTC_READ_NTP;
		#endif
		break;

	case RTC_READ_NTP:
		#ifdef CALENDAR_USE_NTP
		res = Ntp_get_time(timestruct, timestruct->timezone);

		if ( res != NO_RESULT) {
			if (res == S_DONE) {
				Calendar_ToString(str);
				dprintf(PSTR("Get ntp time=<%s>\n"), str);
				Calendar_RemoteUpdate(); // �������������� ����� ntp � sim900
			}

			state_GetRemoteTime = RTC_OK;
		}

		#else
		state_GetRemoteTime = RTC_OK;
		#endif
		break;

	case RTC_OK:
		state_GetRemoteTime = RTC_READ_REMOTE;
		result = true;
		break;

	default:
		break;
	}

	if (res == S_DONE) { _synchronized = true; }

	return result;
}
*/
//SIM900 SetREmote time
/*

bool SetRemoteTime(TDateTime* timestruct) {
	static u08 state = 0;
	static char time[14];
	bool result = false;
	TResult res;

	switch (state) {
	case 0:
		Calendar_ToString(time);
		state = 1;
		break;

	case 1:
		res = Sim900_SetTime(timestruct, 0);

		if ( res != NO_RESULT) {
			if (res == S_DONE) { state = 2; }
			else { state = 3; }
		};

		break;

	case 2:
		dprintf(PSTR("set remote time success\n"));
		state = 0;
		result = true;
		F_need_to_set_remote_time = false;
		break;

	case 3:
		dprintf(PSTR("set remote time abort\n"));
		state = 0;
		result = true;
		break;

	default:
		break;
	}

	return result;
}*/
