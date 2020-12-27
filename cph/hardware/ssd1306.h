#pragma once

namespace cph {
	struct SSD1306_Color {
	private:
		enum class Colors {
			DISPLAY_COLOR_BLACK = 0x00, /*!< Black color, no pixel */
			DISPLAY_COLOR_WHITE = 0x01  /*!< Pixel is set. Color depends on LCD */
		};
	public:
		Colors value;
		static SSD1306_Color Black() {
			return { Colors::DISPLAY_COLOR_BLACK };
		}
		static SSD1306_Color White() {
			return { Colors::DISPLAY_COLOR_WHITE };
		}
		SSD1306_Color Inverted() const {
			return value == Colors::DISPLAY_COLOR_BLACK ? White() : Black();
		}
		bool operator==(const SSD1306_Color& other)const {
			return value == other.value;
		}
	};
	
}

#if defined(WIN32)
	#include "ssd1306_pc.h"
#elif defined(_AVR) || defined(_ARM)
	#include "ssd1306_embed.h"

#endif

