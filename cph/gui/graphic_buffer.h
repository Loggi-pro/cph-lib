#pragma once
#include <cph/containers/bit_field.h>
#include <string.h>
#include <void/math.h>
#include "fonts.h"
#ifdef WIN32
	#include <cstring> //memset
#endif
#ifdef _ARM
	#include <string.h> //memset and string functions
#endif


namespace cph {
	template <uint32_t WIDTH, uint32_t HEIGHT>
	struct BufferTwoColored {
			bit_array_t<WIDTH* HEIGHT> _data;
		private:
			uint16_t _get1DIndex(uint16_t x, uint16_t y) {
				uint8_t n_bit = y % 8;
				uint16_t n_byte = (uint16_t)(x + (y / 8) * WIDTH);
				return  n_byte * 8 + /*bit in byte*/ n_bit;
			}
		public:
			static inline constexpr uint16_t Width = WIDTH;
			static inline constexpr uint16_t Height = HEIGHT;
			template <typename DisplayColor>
			void fill(DisplayColor color) {
				memset(_data, (color == color.Black()) ? 0x00 : 0xFF, sizeof(_data));
			}
			template <typename DisplayColor>
			inline void set(uint16_t x, uint16_t y, DisplayColor color) {
				/* Set color */
				if (color == color.White()) {
					cph::bitfield_t::setBit(_data, _get1DIndex(x, y));
				} else {
					cph::bitfield_t::clearBit(_data, _get1DIndex(x, y));
				}
			}
			void invert(uint16_t x, uint16_t y) {
				for (u16 i = 0; i < sizeof(_data); i++) {
					_data[i] = ~_data[i];
				}
			}
			uint8_t* getPtr() {
				return _data;
			}
	};

	template <typename BUFFER_TYPE, typename DisplayColor>
	struct graphic_buffer_t {
			struct buffer_info_t {
				uint16_t CurrentX;
				uint16_t CurrentY;
				bool Inverted;
				buffer_info_t(): CurrentX(0), CurrentY(0), Inverted(false) {}
			};

			buffer_info_t _buffer_info;
			BUFFER_TYPE _buffer;
			char _getCharIndex(char ch) {
				return (char)(ch - 32);
				/*
					if (ch == ' ') { return 0; }

					if (ch == '%') { return 1; }

					if ((ch >= '0') && (ch <= '9')) {
						return ch - '0' + 2;
					}

					if (ch == 'F') { return 12; }

					if (ch == 'R') { return 13; }

					if (ch == 'X') { return 14; }

					if (ch == 'i') { return 15; }

					if (ch == 'o') { return 16; }

					return 0;*/
			}
			//Transform XY coord to 1d (with PAGE type NAVIGATION)


		public:
			graphic_buffer_t() {}

			const uint8_t* getBuffer() {
				return _buffer.getPtr();
			}

			//Toggles pixels invertion inside internal RAM
			void invertColor() {
				/* Toggle invert */
				_buffer_info.Inverted = !_buffer_info.Inverted;
				/* Do memory toggle */
				_buffer.invert();
			}

			void clear () {
				fill(DisplayColor::Black());
				gotoXY(0, 0);
			}
			//Fills entire LCD with desired color
			void fill(DisplayColor color) {
				_buffer.fill(color);
			}
			//Draws pixel at desired location
			void drawPixel(u16 x, u16 y, DisplayColor color) {
				if (x >= _buffer.Width || y >= _buffer.Height) {
					return;/* Error */
				}

				/* Check if pixels are inverted */
				if (_buffer_info.Inverted) {
					color = color.Inverted();
				}

				_buffer.set(x, y, color);
			}
			//Sets cursor pointer to desired location for strings
			void gotoXY(uint16_t x, uint16_t y) {
				_buffer_info.CurrentX = x;
				_buffer_info.CurrentY = y;
			}
			//Puts character to internal RAM
			char putc(char ch, const font_t* Font, DisplayColor color) {
				/* Check available space in LCD */
				if ( _buffer.Width <= (_buffer_info.CurrentX + Font->FontWidth) ||
				        _buffer.Height <= (_buffer_info.CurrentY + Font->FontHeight)
				   ) {
					/* Error */
					return 0;
				}

				/* Go through font */
				for (uint16_t i = 0; i < Font->FontHeight; i++) {
					//b = Font->data[(ch - 32) * Font->FontHeight + i];
					uint16_t b = Font->data[(uint16_t)_getCharIndex(ch) * Font->FontHeight + i];

					for (uint16_t j = 0; j < Font->FontWidth; j++) {
						if ((uint16_t)(b << j) & 0x8000) {
							drawPixel(_buffer_info.CurrentX + j, (_buffer_info.CurrentY + i), color);
						} else {
							drawPixel(_buffer_info.CurrentX + j, (_buffer_info.CurrentY + i), color.Inverted());
						}
					}
				}

				/*
								// Draw background line over character
								if (_buffer_info.CurrentY > 0) {
									for (j = 0; j < Font->FontWidth; j++) {
										drawPixel(_buffer_info.CurrentX + j, (_buffer_info.CurrentY - 1), (DisplayColor)!color);
									}
								}
				*/
				/* Increase pointer */
				_buffer_info.CurrentX += Font->FontWidth;
				/* Return character written */
				return ch;
			}
			//
			void drawIcon(u16 x0, u16 y0, const font_t* icon, DisplayColor color) {
				_buffer_info.CurrentX = x0;
				_buffer_info.CurrentY = y0 - icon->FontHeight;
				putc(32, icon, color);
			}
			//Puts string to internal RAM
			char puts(const char* str, const font_t* Font, DisplayColor color) {
				/* Write characters */
				while (*str) {
					/* Write character by character */
					if (putc(*str, Font, color) != *str) {
						/* Return error */
						return *str;
					}

					/* Increase string pointer */
					str++;
				}

				/* Everything OK, zero should be returned */
				return *str;
			}
			//Draws line on LCD
			//x0: Line X start point. Valid input is 0 to SSD1306_WIDTH - 1
			//y0: Line Y start point. Valid input is 0 to SSD1306_HEIGHT - 1
			//x1: Line X end point. Valid input is 0 to SSD1306_WIDTH - 1
			//y1: Line Y end point. Valid input is 0 to SSD1306_HEIGHT - 1
			void drawLine(u16 x0, u16 y0, u16 x1, u16 y1, DisplayColor c) {
				s16 dx,dy, sx, sy, err, e2;
				u16 tmp;

				/* Check for overflow */
				if (x0 >= _buffer.Width) {
					x0 = _buffer.Width - 1;
				}

				if (x1 >= _buffer.Width) {
					x1 = _buffer.Width - 1;
				}

				if (y0 >= _buffer.Height) {
					y0 = _buffer.Height - 1;
				}

				if (y1 >= _buffer.Height) {
					y1 = _buffer.Height - 1;
				}

				dx = (x0 < x1) ? (s16)(x1 - x0) : (s16)(x0 - x1);
				dy = (y0 < y1) ? (s16)(y1 - y0) : (s16)(y0 - y1);
				sx = (x0 < x1) ? 1 : -1;
				sy = (y0 < y1) ? 1 : -1;
				err = ((dx > dy) ? dx : -dy) / 2;

				if (dx == 0) {
					if (y1 < y0) {
						tmp = y1;
						y1 = y0;
						y0 = tmp;
					}

					if (x1 < x0) {
						tmp = x1;
						x1 = x0;
						x0 = tmp;
					}

					/* Vertical line */
					for (u16 i = y0; i <= y1; i++) {
						drawPixel(x0, i, c);
					}

					/* Return from function */
					return;
				}

				if (dy == 0) {
					if (y1 < y0) {
						tmp = y1;
						y1 = y0;
						y0 = tmp;
					}

					if (x1 < x0) {
						tmp = x1;
						x1 = x0;
						x0 = tmp;
					}

					/* Horizontal line */
					for (u16 i = x0; i <= x1; i++) {
						drawPixel(i, y0, c);
					}

					/* Return from function */
					return;
				}

				while (true) {
					drawPixel(x0, y0, c);

					if (x0 == x1 && y0 == y1) {
						break;
					}

					e2 = err;

					if (e2 > -dx) {
						err -= dy;
						x0 = (u16)((s16)x0+sx);
					}

					if (e2 < dy) {
						err += dx;
						y0 = (u16)((s16)y0+sy);
					}
				}
			}
			//Draws rectangle on LCD
//x: Top left X start point. Valid input is 0 to SSD1306_WIDTH - 1
//y: Top left Y start point. Valid input is 0 to SSD1306_HEIGHT -
			void drawRectangle(u16 x, u16 y, u16 w, u16 h, DisplayColor c) {
				/* Check input parameters */
				if (
				    x >= _buffer.Width ||
				    y >= _buffer.Height
				) {
					/* Return error */
					return;
				}

				/* Check_buffer.Width and_buffer.Height */
				if ((x + w) >= _buffer.Width) {
					w = _buffer.Width - x;
				}

				if ((y + h) >= _buffer.Height) {
					h = _buffer.Height - y;
				}

				/* Draw 4 lines */
				drawLine(x, y, x + w, y, c);         /* Top line */
				drawLine(x, y + h, x + w, y + h, c); /* Bottom line */
				drawLine(x, y, x, y + h, c);         /* Left line */
				drawLine(x + w, y, x + w, y + h, c); /* Right line */
			}

//  Draws filled rectangle on LCD
//  x: Top left X start point. Valid input is 0 to SSD1306_WIDTH - 1
//  y: Top left Y start point. Valid input is 0 to SSD1306_HEIGHT - 1
//  w: Rectangle_buffer.Width in units of pixels
//  h: Rectangle_buffer.Height in units of pixels
			void drawFilledRectangle(u16 x, u16 y, u16 w, u16 h, DisplayColor c) {
				/* Check input parameters */
				if (
				    x >= _buffer.Width ||
				    y >= _buffer.Height
				) {
					/* Return error */
					return;
				}

				/* Check_buffer.Width and_buffer.Height */
				if ((x + w) >= _buffer.Width) {
					w = _buffer.Width - x;
				}

				if ((y + h) >= _buffer.Height) {
					h = _buffer.Height - y;
				}

				/* Draw lines */
				for (u16 i = 0; i <= h; i++) {
					/* Draw lines */
					drawLine(x, y + i, x + w, y + i, c);
				}
			}
			//
			void drawTriangle(u16 x1, u16 y1, u16 x2, u16 y2, u16 x3, u16 y3, DisplayColor color) {
				/* Draw lines */
				drawLine(x1, y1, x2, y2, color);
				drawLine(x2, y2, x3, y3, color);
				drawLine(x3, y3, x1, y1, color);
			}

			///**
// Draws triangle on LCD
// @ref TM_SSD1306_UpdateScreen() must be called after that in order to see updated LCD screen
// x1: First coordinate X location. Valid input is 0 to SSD1306_WIDTH - 1
// y1: First coordinate Y location. Valid input is 0 to SSD1306_HEIGHT - 1
// x2: Second coordinate X location. Valid input is 0 to SSD1306_WIDTH - 1
// y2: Second coordinate Y location. Valid input is 0 to SSD1306_HEIGHT - 1
// x3: Third coordinate X location. Valid input is 0 to SSD1306_WIDTH - 1
// y3: Third coordinate Y location. Valid input is 0 to SSD1306_HEIGHT - 1
			void drawFilledTriangle(u16 x1, u16 y1, u16 x2,
			                        u16 y2, u16 x3, u16 y3, DisplayColor color) {
				s16 deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0,
				    yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0,
				    curpixel = 0;
				deltax = vd::Abs(x2 - x1);
				deltay = vd::Abs(y2 - y1);
				x = x1;
				y = y1;

				if (x2 >= x1) {
					xinc1 = 1;
					xinc2 = 1;
				} else {
					xinc1 = -1;
					xinc2 = -1;
				}

				if (y2 >= y1) {
					yinc1 = 1;
					yinc2 = 1;
				} else {
					yinc1 = -1;
					yinc2 = -1;
				}

				if (deltax >= deltay) {
					xinc1 = 0;
					yinc2 = 0;
					den = deltax;
					num = deltax / 2;
					numadd = deltay;
					numpixels = deltax;
				} else {
					xinc2 = 0;
					yinc1 = 0;
					den = deltay;
					num = deltay / 2;
					numadd = deltax;
					numpixels = deltay;
				}

				for (curpixel = 0; curpixel <= numpixels; curpixel++) {
					drawLine(x, y, x3, y3, color);
					num += numadd;

					if (num >= den) {
						num -= den;
						x += xinc1;
						y += yinc1;
					}

					x += xinc2;
					y += yinc2;
				}
			}

//Draws circle to STM buffer
//  @ref TM_SSD1306_UpdateScreen() must be called after that in order to see updated LCD screen
//  x: X location for center of circle. Valid input is 0 to SSD1306_WIDTH - 1
//  y: Y location for center of circle. Valid input is 0 to SSD1306_HEIGHT - 1
//  r: Circle radius in units of pixels
			void drawCircle(s16 x0, s16 y0, s16 r, DisplayColor c) {
				s16 f = 1 - r;
				s16 ddF_x = 1;
				s16 ddF_y = -2 * r;
				s16 x = 0;
				s16 y = r;
				drawPixel(x0, y0 + r, c);
				drawPixel(x0, y0 - r, c);
				drawPixel(x0 + r, y0, c);
				drawPixel(x0 - r, y0, c);

				while (x < y) {
					if (f >= 0) {
						y--;
						ddF_y += 2;
						f += ddF_y;
					}

					x++;
					ddF_x += 2;
					f += ddF_x;
					drawPixel(x0 + x, y0 + y, c);
					drawPixel(x0 - x, y0 + y, c);
					drawPixel(x0 + x, y0 - y, c);
					drawPixel(x0 - x, y0 - y, c);
					drawPixel(x0 + y, y0 + x, c);
					drawPixel(x0 - y, y0 + x, c);
					drawPixel(x0 + y, y0 - x, c);
					drawPixel(x0 - y, y0 - x, c);
				}
			}

//Draws filled circle to STM buffer
//x: X location for center of circle. Valid input is 0 to SSD1306_WIDTH - 1
//y: Y location for center of circle. Valid input is 0 to SSD1306_HEIGHT - 1
//r: Circle radius in units of pixels
			void drawFilledCircle(s16 x0, s16 y0, s16 r, DisplayColor c) {
				s16 f = 1 - r;
				s16 ddF_x = 1;
				s16 ddF_y = -2 * r;
				s16 x = 0;
				s16 y = r;
				drawPixel(x0, y0 + r, c);
				drawPixel(x0, y0 - r, c);
				drawPixel(x0 + r, y0, c);
				drawPixel(x0 - r, y0, c);
				drawLine(x0 - r, y0, x0 + r, y0, c);

				while (x < y) {
					if (f >= 0) {
						y--;
						ddF_y += 2;
						f += ddF_y;
					}

					x++;
					ddF_x += 2;
					f += ddF_x;
					drawLine(x0 - x, y0 + y, x0 + x, y0 + y, c);
					drawLine(x0 + x, y0 - y, x0 - x, y0 - y, c);
					drawLine(x0 + y, y0 + x, x0 - y, y0 + x, c);
					drawLine(x0 + y, y0 - x, x0 - y, y0 - x, c);
				}
			}

			void drawImage(const uint8_t* img, uint8_t frame, uint8_t x, uint8_t y) {
				uint32_t i, b, j;
				b = 0;

				if (frame >= img[2])
				{ return; }

				uint32_t start = (frame * (img[3] + (img[4] << 8)));

				/* Go through font */
				for (i = 0; i < img[1]; i++) {
					for (j = 0; j < img[0]; j++) {
						drawPixel(x + j, (y + i), (uint8_t) (img[b / 8 + 5 + start] >> (b % 8)) & 1);
						b++;
					}
				}
			}

	};





}





