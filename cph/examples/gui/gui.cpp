#include <void/chrono.h>
#include <string.h>
#include "gui.h"

uint32_t FTemperature = 25;
uint32_t FCurrentSpeed = 120;
uint32_t FDelay = 100;

//#include <cph/system_counter.h>
/* reverse:  �������������� ������ s �� ����� */
static void reverse(char s[]) {
	for (size_t i = 0, j = strlen(s) - 1; i < j; i++, j--) {
		char c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

/* itoa:  ������������ n � ������� � s */
static void itoa(s16 n, char s[]) {
	uint8_t i;
	s08 sign = 1;

	if (n < 0) { /* ���������� ���� */
		sign = -1;
		n = -n;
	}          /* ������ n ������������� ������ */

	i = 0;

	do {       /* ���������� ����� � �������� ������� */
		s[i++] = n % 10 + '0';   /* ����� ��������� ����� */
	} while ((n /= 10) > 0);     /* ������� */

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



cph::TTextBlock<Gui::DisplayType> Gui::tb_KimLight = cph::TTextBlock<DisplayType>::Builder{ &Gui::Parent::DISPLAY, cph::FONTS_T::Font11x18, &Gui::getStringInit }
        .setPosition({ 0, 0 })
        .setSize({ DisplayType::Width, DisplayType::Height })
        .setAlign(cph::ALIGNMENT_CENTER, cph::VALIGNMENT_CENTER)
        .setColor(Gui::DisplayType::DisplayColor::White())
        .create();

cph::TIcon<Gui::DisplayType> Gui::iconLocked = cph::TIcon<DisplayType>::Builder{ &Gui::Parent::DISPLAY, cph::GLYPHS_T::GLYPH_LOCK_7x10 }
        .setPosition({ 10, 10 })
        .create();


cph::TNoBorderForm<Gui::DisplayType> Gui::_initForm = cph::TNoBorderForm< Gui::DisplayType>::Builder{ &Gui::Parent::DISPLAY }
        .setChildren<Gui::tb_KimLight, Gui::iconLocked>()
        .create();


cph::TCaptionForm<Gui::DisplayType> Gui::_mainForm = cph::TCaptionForm< Gui::DisplayType>::Builder{ &Gui::Parent::DISPLAY,
                                         cph::TTextBlock<DisplayType>::Builder{&Gui::Parent::DISPLAY, cph::FONTS_T::Font7x10, &Gui::getCaption}
                                         .setPosition({0, 0})
                                         .setSize({Gui::DisplayType::Width, 12})
                                         .setAlign(cph::ALIGNMENT_CENTER, cph::VALIGNMENT_CENTER)
                                         .setColor(Gui::DisplayType::DisplayColor::Black())
                                         .create()
                                                                                                  }
                                         .setChildren< Gui::tbTemperature, Gui::tbCurrentSpeed>()
                                         .create();


cph::TTextBlock<Gui::DisplayType> Gui::tbTemperature = cph::TTextBlock<Gui::DisplayType>::Builder{ &Gui::Parent::DISPLAY, cph::FONTS_T::Font7x10, &Gui::getStringTemperature }
        .setPosition({ Gui::DisplayType::Width - 38, 1 })
        .setSize({ 45, 10 })
        .setAlign(cph::ALIGNMENT_RIGHT, cph::VALIGNMENT_TOP)
        .setColor(Gui::DisplayType::DisplayColor::Black())
        .create();

cph::TTextBlock<Gui::DisplayType> Gui::tbCurrentSpeed = cph::TTextBlock<Gui::DisplayType>::Builder{ &Gui::Parent::DISPLAY, cph::FONTS_T::Font11x18, &Gui::getStringSpeed }
        .setPosition({ 0, 5 })
        .setSize({ DisplayType::Width,  DisplayType::Height })
        .setAlign(cph::ALIGNMENT_CENTER, cph::VALIGNMENT_CENTER)
        .setColor(Gui::DisplayType::DisplayColor::White())
        .create();




