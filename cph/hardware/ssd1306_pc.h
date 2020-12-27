#pragma once
#include <void/gsl.h>
#include <cph/gui.h>
#include <cph/async.h>
#include "sfml_graphics.h"
#include <cph/containers/bit_field.h>
namespace cph {
	template <class I2C_DUMMY = void, uint8_t SSD1306_I2C_ADDR_DUMMY = 0x00, uint32_t SSD_WIDTH = 128, uint32_t SSD_HEIGHT = 64, typename enablePin = void>
	struct ssd1306_t : public graphic_buffer_t<BufferTwoColored<SSD_WIDTH, SSD_HEIGHT>, SSD1306_Color> {
		private:
			using Parent = graphic_buffer_t<BufferTwoColored<SSD_WIDTH, SSD_HEIGHT>, SSD1306_Color>;
			int _isNeedUpdate = 0;
			bool _isInUpdateMode = false;
			bool _isInited = false;
			TGraphicWindow* Fbitmap = nullptr;
#pragma pack(push,1)
			struct TColor32bit {
				u08 r;
				u08 g;
				u08 b;
				u08 alpha;
			};
#pragma pack(pop)
			const TColor32bit ColorWhite { 255, 255, 255, 255 };
			const TColor32bit ColorBlack{ 0, 0, 0, 255 };
			const TColor32bit ColorGreen{ 100, 250, 140, 255 };

			template <typename T>
			uint8_t* convert_bitmap_from_page_to_conseq(const T SSD1306_buffer) {
				//convert SSD1306 buffer to standart bitmap
				const int PAGE_HEIGHT = vd::min(Height, uint32_t(8));
				const int PAGE_WIDTH = Width;
				static uint8_t pixels[Width * Height * sizeof(TColor32bit)];
				TColor32bit* pix = (TColor32bit*)pixels;

				for (int i = 0; i < Width * Height; i++) {
					TColor32bit color = cph::bitfield_t::isBitSet(SSD1306_buffer, i) ? ColorGreen : ColorBlack;
					int inside_page = i % (PAGE_WIDTH * PAGE_HEIGHT);
					int x = inside_page / PAGE_HEIGHT;
					int y = inside_page % PAGE_HEIGHT;
					int page = i / (PAGE_WIDTH * PAGE_HEIGHT);
					int d = x + y * PAGE_WIDTH + page * PAGE_WIDTH * PAGE_HEIGHT;
					pix[d] = color;
				}

				return pixels;
			}

		public:
			static constexpr inline uint32_t Width = SSD_WIDTH;
			static constexpr inline uint32_t Height = SSD_HEIGHT;
			using DisplayColor = SSD1306_Color;
			void RequestForUpdate() {
				if (_isNeedUpdate < 2) { _isNeedUpdate++; }
			}
			bool isNeedUpdate()const {
				return _isNeedUpdate > 0;
			}

			bool isInUpdateMode() const {
				return _isInUpdateMode;
			}
			bool isInited()const {
				return _isInited;
			}
			ssd1306_t() {}

			cph::TVoidResultAsync init() {
				static uint8_t state = 0;
				state++;

				if (state < 10) { return {}; }

				state = 0;

				if (Fbitmap == nullptr) {
					const uint32_t SCALE = 800 / (vd::max(Width, Height));
					Fbitmap = new TGraphicWindow("SSD1306", Width, Height, SCALE);
					_isInited = true;
					return { true };
				}

				return {};
			}

			void on() {}
			void off() {}

			// Implements reset procedure, toggles reset pin, uses dump delay
			cph::TVoidResultAsync hardReset() {
				return true;
			}

			cph::TVoidResultAsync updateScreen() {
				static uint8_t state = 0;
				//assert(Fbitmap != nullptr); //"SSD1306 not inited"

				if (state == 0) {
					_isInUpdateMode = true;
					state++;
				} else if (state < 8) {
					state++;
				} else {
					Fbitmap->draw(convert_bitmap_from_page_to_conseq(Parent::_buffer._data));
					state = 0;

					if (_isNeedUpdate > 0) { _isNeedUpdate--; }

					if (_isNeedUpdate == 0) {
						_isInUpdateMode = false;
						return true;
					}
				}

				return {};
			}
	};

}
