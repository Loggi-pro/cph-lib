#pragma once
#include <void/geometry.h>
#include <cph/timer.h>
#include <cph/async.h>
#include "components.h"
namespace cph {
	template <typename T>
	class AbstractGui {
		public:
			static inline T DISPLAY;
		private:
			void _blink(cph::TTextBlock<T>& tb) {
				static cph::timer_t timer;

				if (timer.isElapsed(500_ms)) {
					tb.setColor(tb.getColor().Inverted());
				}
			}
			cph::TAbstractForm<T>* _currentForm = nullptr;

			cph::timer_t _guiTimer = {};
			void _changeForm(cph::TAbstractForm<T>* newForm) {
				if (_currentForm != nullptr) {
					_currentForm->visibility(false);
				}

				_currentForm = newForm;
				_currentForm->visibility(true);
				_currentForm->draw();
				DISPLAY.RequestForUpdate();
			}
		public:
			AbstractGui() {
				_guiTimer.start();
			}
			cph::Result<void> init() {
				if (!DISPLAY.isInited()) {
					DISPLAY.init();
					return {};
				} else {
					auto res = DISPLAY.updateScreen();
					return res;
				}
			}
			Result<void> update() {
				if (!DISPLAY.isNeedUpdate()) return {true};

				return DISPLAY.updateScreen();
			}
			void deinit() {
			}
			void poll() {
				if (_currentForm == nullptr) { return; }

				if (_guiTimer.isElapsed(vd::chrono::milli_t(40)) && !DISPLAY.isInUpdateMode()) {
					_currentForm->draw();
				}

				if (DISPLAY.isNeedUpdate()) {
					DISPLAY.updateScreen();
				}
			}
			void enable() {
				DISPLAY.on();
			}
			void disable() {
				DISPLAY.off();
			}
			void setScreen(cph::TAbstractForm<T>* newForm) {
				if (_currentForm != newForm) {
					_changeForm(newForm);
				}
			}
	};
}

/*

	using DisplayType = cph::ssd1306_t<void, 0, 128, 64>;

class Gui:public cph::AbstractGui<DisplayType> {
	using DisplayType = DisplayType;
	using Parent = cph::AbstractGui<DisplayType>;
	static bool getStringInit(cph::TTextBlock<DisplayType>* self, char* value);
	static bool getStringTemperature(cph::TTextBlock<DisplayType>* self, char* value);
	static bool getStringSpeed(cph::TTextBlock<DisplayType>* self, char* value);
	static bool getCaption(cph::TTextBlock<DisplayType>* self, char* value);
	static cph::TTextBlock<DisplayType> tb_KimLight;
	static cph::TTextBlock<DisplayType> tbTemperature;
	static cph::TTextBlock<DisplayType> tbCurrentSpeed;
	static cph::TIcon<DisplayType> iconLocked;
public:
	static cph::TNoBorderForm<DisplayType> _initForm;
	static cph::TCaptionForm< DisplayType> _mainForm;

	Gui(){}

};

#include "gui.h"
#include <string.h>
#include <math.h>
#include "settings.h"
#include <void/chrono.h>
#include <cph/graphics/glyphs.h>
//#include <cph/system_counter.h>

static void reverse(char s[]) {
	for (size_t i = 0, j = strlen(s) - 1; i < j; i++, j--) {
		char c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

static void itoa(s16 n, char s[]) {
	u08 i;
	s08 sign = 1;

	if (n < 0) {
		sign = -1;
		n = -n;
	}

	i = 0;

	do {
		s[i++] = n % 10 + '0';
	} while ((n /= 10) > 0);

	if (sign < 0) {
		s[i++] = '-';
	}

	s[i] = '\0';
	reverse(s);
}


bool Gui::getStringInit(cph::TTextBlock<DisplayType>* self, char* value) {
	if (!self->is_inited()) {
		strcpy(value, "LOADING");
		return true;
	}

	return false;
}


bool Gui::getCaption(cph::TTextBlock<DisplayType>* self, char* value) {
	if (!self->is_inited()) {
		strcpy(value, "Menu");
		return true;
	}

	return false;
}


bool Gui::getStringTemperature(cph::TTextBlock<DisplayType>* self, char* out_string) {
	static int16_t lCachedTemperature = 0;
	int16_t lTemperature = FTemperature;

	if (!self->is_inited() || lCachedTemperature != lTemperature) {
		lCachedTemperature = lTemperature;
		strcpy(out_string, "t:");
		itoa(lCachedTemperature, out_string + strlen("t:"));
		strcat(out_string, "'C ");
		return true;
	}

	return false;
}

bool Gui::getStringSpeed(cph::TTextBlock<DisplayType>* self, char* value) {
	static uint32_t FtempSpeed = 0;
	uint32_t lCurrentSpeed = FCurrentSpeed;

	if (!self->is_inited() || FtempSpeed != lCurrentSpeed) {
		FtempSpeed = lCurrentSpeed;
		itoa(FtempSpeed, value);
		strcat(value, " km/h");
		return true;
	}

	return false;
}



cph::TTextBlock<DisplayType> Gui::tb_KimLight = cph::TTextBlock<DisplayType>::Builder{&Gui::Parent::DISPLAY, cph::FONTS_T::Font11x18,&Gui::getStringInit }
	.setPosition({ 0, 0 })
	.setSize({ DisplayType::Width, DisplayType::Height })
	.setAlign(cph::ALIGNMENT_CENTER, cph::VALIGNMENT_CENTER)
	.setColor(Gui::DisplayType::DisplayColor::White())
	.create();

cph::TIcon<DisplayType> Gui::iconLocked = cph::TIcon<DisplayType>::Builder{&Gui::Parent::DISPLAY, cph::GLYPHS_T::GLYPH_LOCK_7x10 }
	.setPosition({10,10})
	.create();


cph::TNoBorderForm<Gui::DisplayType> Gui::_initForm = cph::TNoBorderForm< Gui::DisplayType>::Builder{&Gui::Parent::DISPLAY}
	.setChildren<Gui::tb_KimLight,Gui::iconLocked>()
	.create();


cph::TCaptionForm<Gui::DisplayType> Gui::_mainForm = cph::TCaptionForm< Gui::DisplayType>::Builder{&Gui::Parent::DISPLAY,
		cph::TTextBlock<DisplayType>::Builder{&Gui::Parent::DISPLAY,cph::FONTS_T::Font7x10,&Gui::getCaption}
		.setPosition({0,0})
		.setSize({Gui::DisplayType::Width,12})
		.setAlign(cph::ALIGNMENT_CENTER,cph::VALIGNMENT_CENTER)
		.setColor(Gui::DisplayType::DisplayColor::White())
		.create()
	}
	.setChildren< Gui::tbTemperature, Gui::tbCurrentSpeed>()
	.create();


cph::TTextBlock<Gui::DisplayType> Gui::tbTemperature = cph::TTextBlock<Gui::DisplayType>::Builder{&Gui::Parent::DISPLAY, cph::FONTS_T::Font7x10,&Gui::getStringTemperature }
	.setPosition({ 2, 2 })
	.setSize({ DisplayType::Width, 12 })
	.setAlign(cph::ALIGNMENT_RIGHT, cph::VALIGNMENT_TOP)
	.setColor(Gui::DisplayType::DisplayColor::White())
	.create();

cph::TTextBlock<Gui::DisplayType> Gui::tbCurrentSpeed = cph::TTextBlock<Gui::DisplayType>::Builder{&Gui::Parent::DISPLAY, cph::FONTS_T::Font11x18,&Gui::getStringSpeed }
	.setPosition({ 0, 5 })
	.setSize({ DisplayType::Width,  DisplayType::Height })
	.setAlign(cph::ALIGNMENT_CENTER, cph::VALIGNMENT_CENTER)
	.setColor(Gui::DisplayType::DisplayColor::White())
	.create();


	Gui gui;
	while (!gui.init()) continue;
	gui.setScreen(&Gui::_initForm);
	while (!gui.update()) continue;
	vd::delay_ms<2000>();
	gui.setScreen(&Gui::_mainForm);
	gui.update();
	while (true) {
		gui.poll();
	}

*/