#pragma once
#include <cph/timer.h>
#include <void/optional.h>
#include <void/chrono.h>
#include <void/meta.h>
// The module provides jitter (bounce) elimination and performs encoder and buttons polling.


// Key events:
// On key down		-	Triggers immediately after a button is pushed
// On key up		-	Triggers immediately after a button is released
// On key long		-	Triggers with KEY_LONG_DELAY after a button is pushed
// On key sequential-	First time triggers with delay KEY_SEQUENTAL_DELAY_FIRST after button
//						is pushed, then triggers sequentially with period KEY_SEQUENTAL_DELAY_PERIOD
//						while the button is still pushed.


#define KEY_LONG_DELAY			  1200_ms
#define KEY_SEQUENTAL_DELAY_FIRST  800_ms
#define KEY_SEQUENTAL_DELAY_PERIOD 100_ms
#define BUTTONS_SCAN_DELAY 50_ms

namespace cph {
	enum KeyEventType { NO_KEY_EVENT, ON_KEY_DOWN, ON_KEY_UP, ON_KEY_LONG, ON_KEY_SEQUENTIAL};

	namespace Private {

		class buttonActions_t {

				static bool _isOnKeyLongEvent(KeyEventType event, cph::timer_t& timer) {
					if (event != ON_KEY_DOWN) { return false; }

					if (timer.isElapsed(KEY_LONG_DELAY)) {
						return true;
					}

					return false;
				}
				static bool _isOnKeySequeintialEvent(KeyEventType event, cph::timer_t& timer,
				                                     vd::chrono::milli_t* seqDelay) {
					if (event != ON_KEY_LONG && event != ON_KEY_SEQUENTIAL) { return false; }

					if (timer.isElapsed(*seqDelay)) {
						*seqDelay = KEY_SEQUENTAL_DELAY_PERIOD;
						timer.start();
						return true;
					}

					return false;
				}
			public:
				static vd::optional_t<KeyEventType> _getEvent(KeyEventType event, cph::timer_t& timer,
				        vd::chrono::milli_t* seqDelay, bool IsPressed) {
					if (!IsPressed) {
						if (event == NO_KEY_EVENT) return vd::optional_t<KeyEventType> {}; //no event

						if (event != ON_KEY_UP) {
							return ON_KEY_UP;
						} else {
							//disable all;
							*seqDelay = KEY_SEQUENTAL_DELAY_FIRST;
							return NO_KEY_EVENT;
						}
					} else {//button pressed
						if (event == NO_KEY_EVENT) {
							timer.start();
							return ON_KEY_DOWN;
						}

						//check that is long

						if (_isOnKeyLongEvent(event, timer)) { return ON_KEY_LONG; }

						if (_isOnKeySequeintialEvent(event, timer, seqDelay)) { return ON_KEY_SEQUENTIAL; }
					}

					return vd::optional_t<KeyEventType>(); //no event
				}

		};

	}
//Базовый класс кнопки (описывает все действия)
	template <int ID, typename PARENT>
	class base_button_t: private Private::buttonActions_t {
			static inline cph::timer_t _timerButton;
			static inline KeyEventType _event = NO_KEY_EVENT;
			static inline vd::chrono::milli_t _sequentalDelay = KEY_SEQUENTAL_DELAY_FIRST;
		public:
			base_button_t() = delete;
			constexpr static inline int id = ID;
			static inline bool isBlocked = false;
			static void init() {_timerButton.start();}

			static bool IsPressed() = delete;
			static vd::optional_t<KeyEventType> scan() {
				if (isBlocked) {
					_sequentalDelay = KEY_SEQUENTAL_DELAY_FIRST;
					_timerButton.start();
					return vd::optional_t<KeyEventType>();
				}

				auto t = Private::buttonActions_t::_getEvent(_event, _timerButton, &_sequentalDelay,
				         PARENT::IsPressed());

				if (t) {_event = t.value;}

				return t;
			}
	};

//класс кнопки ( инкапсулирует чтение )
	template<typename pin, int ID>
	class button_t: public base_button_t<ID, button_t<pin, ID>> {
		public:
			button_t() = delete;
			using base_button_t<ID, button_t<pin, ID>>::id;
			static void init() {
				base_button_t<ID, button_t<pin, ID>>::init();
				pin::Port::Enable();
				pin::SetConfiguration(pin::Port::In);
			}

			static bool IsPressed() {
				return !pin::IsSet();
			}
	};
	/*
	//виртуальная кнопка
	template<int ID,typename PARENT>
	class vbutton_t: public base_button_t<ID,PARENT> {
			vbutton_t() = delete;
		public:
		using  base_button_t<ID,PARENT>::id;
			static void init() = delete;
			static bool IsPressed() = delete;
	};
	*/
	template <int ID, typename button1, typename ...bs>
	class complexbutton_t: public base_button_t<ID, complexbutton_t<ID, button1, bs...>> {
			complexbutton_t() = delete;
			using buttons = vd::fn::list<button1, bs...>;
		public:
			static void init () {
				vd::fn::

				foreach (buttons{}, [](auto b) {
				typedef typename decltype(b)::type b_t;
					b_t::init();
				});
			}
			static bool IsPressed() {
				bool res = vd::fn::all_of(buttons{}, [](auto b) {
					typedef typename decltype(b)::type b_t;
					return b_t::IsPressed();
				});
				vd::fn::

				foreach (buttons{}, [](auto b, bool result) {
				typedef typename decltype(b)::type b_t;
					b_t::isBlocked = result;
				}, res);
				return res;
			}

	};



	struct TKeyEvent {
		KeyEventType eventType;
		int ID;
		bool handled;
	};



	template <typename button1, typename ...buttons>
	class buttons_t {
			static inline cph::timer_t _scanTimer;

			static vd::optional_t<TKeyEvent> _scanKeys(void) {
				TKeyEvent ev = {NO_KEY_EVENT, -1, false};
				bool finded = false;
				auto l = [](auto t, bool * finded, TKeyEvent * ev) {
					typedef typename decltype(t)::type button_type; //button_type = button_handler

					if (auto t = button_type::scan()) {
						ev->eventType = t.value;
						ev->ID = button_type::id;
						*finded = true;
						return true;
					}

					return false;
				};
				vd::fn::apply_till(vd::fn::list<button1, buttons...> {}, l, &finded, &ev);

				if (finded) { return ev; }
				else return vd::optional_t<TKeyEvent> {};
			}

		public:
			static void init() {
				_scanTimer.start();
				auto l = [](auto t) {
					typedef typename decltype(t)::type button_type; //button_type = button_handler
					button_type::init();
				};
				vd::fn::foreach(vd::fn::list<button1, buttons...> {}, l);
			}
			static vd::optional_t<TKeyEvent> scan() {
				if (!_scanTimer.isElapsed(100_ms)) { return vd::optional_t<TKeyEvent>(); }

				return _scanKeys();
			}
			buttons_t() = delete;
	};

}
