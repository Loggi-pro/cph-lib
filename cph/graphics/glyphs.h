#pragma once
#include "fonts.h"
/**
//brief  7 x 10 pixels font size structure
*/
namespace cph {
	struct GLYPHS_T {
		private:
		static const u16 _GLYPH_LOCK_7x10[];
		public:
		inline static const font_t GLYPH_LOCK_7x10 = {7,10,_GLYPH_LOCK_7x10};
	};
	
	inline const u16 GLYPHS_T::GLYPH_LOCK_7x10 [] = {
		0x3800, 0x4400, 0x4400, 0xFE00, 0xC600, 0xEE00, 0xEE00, 0xFE00, 0x0000, 0x0000
	}
	
	
}
