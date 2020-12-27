#pragma once
#include <cph/hardware/ssd1306.h>
#include <cph/timer.h>
#include <void/chrono.h>
#include <cph/gui.h>

extern uint32_t FTemperature;
extern uint32_t FCurrentSpeed;
extern uint32_t FDelay;

struct i2c_fake {
	static bool writeByte(uint8_t, uint8_t, uint8_t) {
		return true;
	};
	static bool writeData(uint8_t, uint8_t, const vd::gsl::span<uint8_t>&) {
		return true;
	}
};

using DisplayTypeGlobal = cph::ssd1306_t<i2c_fake, 0, 128, 64>;

class Gui : public cph::AbstractGui<DisplayTypeGlobal> {
		using DisplayType = DisplayTypeGlobal;
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

		Gui() {}

};