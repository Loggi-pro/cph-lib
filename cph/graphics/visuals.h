#pragma once
#include "geometry.h"
#include "fonts.h"
#include <void/gsl.h>
namespace cph {
	template <typename T, T& DISPLAY>
	class TForm;
	template <typename T, T& DISPLAY>
	class TTextBlock;
	template <typename T, T& DISPLAY>
	using tb_getValuePtr = bool (*)(TTextBlock<T, DISPLAY>* self, char* value);
	template <typename T, T& DISPLAY>
	using f_onEventPtr = bool (*)(TForm<T, DISPLAY>* self);
	template <typename T, T& DISPLAY>
	using f_onEventCheckPtr = bool (*)(TForm<T, DISPLAY>* self);
	enum TEXT_ALIGNMENT { ALIGNMENT_LEFT, ALIGNMENT_CENTER, ALIGNMENT_RIGHT };
	enum TEXT_VALIGNMENT { VALIGNMENT_BOTTOM, VALIGNMENT_CENTER, VALIGNMENT_TOP };

	template <typename T, T& DISPLAY>
	struct TElementList {
		TTextBlock<T, DISPLAY>** _el;
		u08 _size;
		TElementList(TTextBlock<T, DISPLAY>** elements, u08 s) : _el(elements), _size(s) {
		}
	};

#define TB_TEXT_LENGTH 20

	template <typename T, T& DISPLAY>
	class TForm {
		public:
			enum TFormType { FORM_BORDERLESS, FORM_CAPTION,FORM_CUSTOM };
		private:
			TFormType _type;
			TTextBlock<T, DISPLAY>* _caption;
			TGeometricSize _size;
			T2dPoint _pos; //left_bottom;
			TElementList<T, DISPLAY> _childs;
			f_onEventPtr<T, DISPLAY> _onDraw;
			f_onEventCheckPtr<T, DISPLAY> _onDrawCheck;
			bool _is_visible;
			bool _is_changed;
			void _redraw() {
				_is_changed = false;

				if (!_is_visible) { return; }

				enum { CAPTION_HEIGHT = 12, TITLE_LEFT_OFFSET = 2 };
				DISPLAY.fill(DISPLAY_COLOR_BLACK);

				switch (_type) {
				case FORM_BORDERLESS:
					break;

				case FORM_CAPTION:
					DISPLAY.drawFilledRectangle(0, 0, _size.Width, CAPTION_HEIGHT,
					                            DISPLAY_COLOR_WHITE);

					if (_caption != nullptr) {
						_caption->draw();
					}

					break;

				case FORM_CUSTOM:
				break;
				default:
					break;
				}

				if (_onDraw != nullptr) {
					(*_onDraw)(this);
				}

				for (u08 i = 0; i < _childs._size; i++) {
					_childs._el[i]->draw();
				}

				// DISPLAY.updateScreen();
				DISPLAY.RequestForUpdate();
			}

		public:

			constexpr TForm(const T2dPoint& p, const  TGeometricSize& size,
			      TElementList<T, DISPLAY> childs):  _type(FORM_BORDERLESS),
				_caption(nullptr), _size(size), _pos(p), _childs(childs), _is_visible(true),
				_is_changed(true) {}
			constexpr TForm(TFormType type, const  T2dPoint& p, const  TGeometricSize& size,
			      TTextBlock<T, DISPLAY>* caption,
			      TElementList<T, DISPLAY> childs, f_onEventPtr<T, DISPLAY> onDraw = nullptr,
			      f_onEventCheckPtr<T, DISPLAY> onDrawCheck = nullptr):  _type(type), _caption(caption), _size(size),
				_pos(p),
				_childs(childs),
				_onDraw(onDraw), _onDrawCheck(onDrawCheck),
				_is_visible(true), _is_changed(true) {}

			void draw() {
				bool is_changed = _is_changed;

				if (_caption != nullptr) {
					_caption->update();
					is_changed = is_changed || _caption->is_changed();
				}

				if (_onDrawCheck != nullptr) {
					is_changed = is_changed || _onDrawCheck(this);
				}

				for (u08 i = 0; i < _childs._size; i++) {
					_childs._el[i]->update();
					is_changed = is_changed || _childs._el[i]->is_changed();
				}

				if (!is_changed) { return; }

				_redraw();
			}
			void visibility(bool is_visible) {
				if (_is_visible == is_visible) { return; }

				_is_visible = is_visible;
				_is_changed = true;
			}
	};

	template <typename T, T& DISPLAY>
	class TTextBlock {
			char _currentValue[TB_TEXT_LENGTH];
			tb_getValuePtr<T, DISPLAY> _value_ptr;
			SSD1306_COLOR_t _color;
			TEXT_ALIGNMENT _h_align;
			TEXT_VALIGNMENT _v_align;
			//TForm* _parent;
			TGeometricSize _size;
			T2dPoint _pos; //left_bottom;
			const font_t& _font;
			bool _is_init = false;
			bool _is_selected;
			bool _is_changed;
			T2dPoint _alignText(u16 width, u16 height, const font_t& font,
			                    const char* str, TEXT_ALIGNMENT h_align = ALIGNMENT_LEFT,
			                    TEXT_VALIGNMENT v_align = VALIGNMENT_CENTER) {
				T2dPoint res;
				font_size_t size = font.getSize(str);

				switch (h_align) {
				case ALIGNMENT_LEFT:
					res.x = 0;
					break;

				case ALIGNMENT_CENTER:
					res.x = (width - size.Length) / 2;
					break;

				case ALIGNMENT_RIGHT:
					res.x = width - size.Length - 1;
					break;

				default:
					break;
				}

				switch (v_align) {
				case VALIGNMENT_BOTTOM:
					res.y = height - size.Height;
					break;

				case VALIGNMENT_CENTER:
					res.y = (height - size.Height) / 2;
					break;

				case VALIGNMENT_TOP:
					res.y = 0;
					break;
				}

				return res;
			}
			void _redraw() {
				_is_changed = false;
				T2dPoint p = _alignText(_size.Width, _size.Height, _font,
				                        _currentValue, _h_align, _v_align);
				SSD1306_COLOR_t text_color = _color;
				SSD1306_COLOR_t bg_color = colorInvert(_color);

				if (_is_selected) {
					SSD1306_COLOR_t t = text_color;
					text_color = bg_color;
					bg_color = t;
				}

				if (bg_color == DISPLAY_COLOR_WHITE) {
					DISPLAY.drawFilledRectangle(_pos.x, _pos.y, _size.Width, _size.Height,
					                            bg_color);
				}

				DISPLAY.gotoXY(_pos.x + p.x, _pos.y + p.y);
				DISPLAY.puts(_currentValue, &_font, text_color);
			}
			void _update_text() {
				if (_value_ptr == nullptr) { return; }

				/*
					if (_value_ptr == &GetString_menuTitle1) {
						_value_ptr = _value_ptr;
					}*/
				bool result = (*_value_ptr)(this,
				                            _currentValue); // must be in separate line from is_changed;
				_is_changed = _is_changed ||  result;
				_is_init = true;
			}
		public:
			constexpr TTextBlock(/*TForm* parent,*/ const T2dPoint& p, const TGeometricSize& size,
			                              tb_getValuePtr<T, DISPLAY> func,
			                              const font_t& f,
			                              TEXT_ALIGNMENT h_align = ALIGNMENT_LEFT,
			                              TEXT_VALIGNMENT v_align = VALIGNMENT_CENTER,
			                              SSD1306_COLOR_t c = DISPLAY_COLOR_WHITE):
				/*_parent(parent),*/
				_value_ptr(func), _color(c), _h_align(h_align), _v_align(v_align), _size(size), _pos(p),
				_font(f), _is_init(false), _is_selected(false), _is_changed(true)
			{}
			void update() {
				_update_text();
			}
			bool is_changed()const {
				return _is_changed;
			}
			bool is_inited()const {
				return _is_init;
			}
			void draw() {
				_redraw();
			}
			void selected(bool flag) {
				if (_is_selected != flag) {
					_is_selected = flag;
					_is_changed = true;
				}
			}
			bool is_selected() const {
				return _is_selected;
			}
			void setFont(const font_t& f) {
				if (&_font != &f) {
					_font = f;
					_is_changed = true;
				}
			}
			void setColor(SSD1306_COLOR_t c) {
				if (c == _color)  { return; }

				_color = c;
				_is_changed = true;
			}
			SSD1306_COLOR_t getColor()const {
				return _color;
			}

	};



	template <typename T, T& DISPLAY>
	class TIcon {
			font_t* _icon;
			bool _is_visible = true;
			bool _is_changed = true;
			T2dPoint _pos; //left_bottom;
			SSD1306_COLOR_t _color;
			void _redraw() {
				_is_changed = false;

				if (_is_visible) {
					DISPLAY.drawIcon(_pos.x, _pos.y, _icon,
					                 _color);
				}
			}
		public:
			TIcon(const T2dPoint& p, font_t* icon,
			      SSD1306_COLOR_t c = DISPLAY_COLOR_WHITE):  _icon(icon), _pos(p), _color(c) {
			}
			//void update();
			bool is_changed() const {
				return _is_changed;
			}
			void draw() {
				_redraw();
			}
			bool is_visible() const {
				return _is_visible;
			}
			void visible(bool is_visible) {
				if (_is_visible != is_visible) {
					_is_visible = is_visible;
					_is_changed = true;
				}
			}

	};
}
