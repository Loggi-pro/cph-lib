#pragma once
#include <void/gsl.h>
#include <void/geometry.h>
#include "fonts.h"
namespace cph {
	

	
	template <typename DisplayType>
	class TAbstractForm;
	template <typename DisplayType>
	class TTextBlock;
	template <typename DisplayType>
	using tb_getValuePtr = bool (*)(TTextBlock<DisplayType>* self, char* value);
	template <typename DisplayType>
	using f_onEventPtr = bool (*)(TAbstractForm<DisplayType>* self);
	template <typename DisplayType>
	using f_onEventCheckPtr = bool (*)(TAbstractForm<DisplayType>* self);
	enum TEXT_ALIGNMENT { ALIGNMENT_LEFT, ALIGNMENT_CENTER, ALIGNMENT_RIGHT };
	enum TEXT_VALIGNMENT { VALIGNMENT_BOTTOM, VALIGNMENT_CENTER, VALIGNMENT_TOP };


#define TB_TEXT_LENGTH 20


struct GeometricSize {
	u16 Width;
	u16 Height;
	GeometricSize(const GeometricSize& other) : Width(other.Width),
		Height(other.Height) {
	}
	GeometricSize(u16 aWidth = 0, u16 aHeight = 0) : Width(aWidth),
		Height(aHeight) {}
};


	template <typename DisplayType>
	class TAbstractForm {
		using Self = TAbstractForm < DisplayType>;
		//friend void _redraw(Self* f);
		//friend class TNoBorderForm<DisplayType>;
	protected:
		using DisplayColor = typename DisplayType::DisplayColor;
		void _redraw() {
			_is_changed = false;
			virtualDraw(this);
			/*
			DISPLAY->fill(DisplayColor::Black());
			if (childrenRedraw != nullptr) {
				childrenRedraw();
			}
			DISPLAY->RequestForUpdate();
			*/
		}
		friend class Builder;
		DisplayType* DISPLAY;
		GeometricSize _size = { DisplayType::Width,DisplayType::Height };
		vd::Point2D<uint16_t> _pos = { 0,0 }; //left_bottom;
		f_onEventCheckPtr<DisplayType> _onDrawCheck = nullptr;
		bool _is_visible = true;
		bool _is_changed = true;

		bool (*childrenUpdateAndCheckChange)() = nullptr;
		void (*childrenRedraw)() = []() {};
		void (*virtualDraw)(Self*);
		typedef void(*draw_func)(Self*);
		constexpr TAbstractForm(DisplayType* display, draw_func drawProcedure):DISPLAY(display), virtualDraw(drawProcedure){}
		template <typename T>
		struct Builder {
		protected:
			T form;
			template <typename ...Args>
			constexpr Builder(Args...args):form(args...) {
			}
		public:
			constexpr T create() {
				return form;
			}
			constexpr Builder<T>& setPosition(const vd::Point2D<uint16_t>& p) {
				form._pos = p;
				return *this;
			}
			constexpr Builder<T>& setSize(const  GeometricSize& size) {
				form._size = size;
				return *this;
			}
			constexpr Builder<T>& setCaption(TTextBlock<DisplayType>* caption) {
				form._caption = *caption;
				return *this;
			}
			constexpr Builder<T>& onDrawCheck(f_onEventPtr<DisplayType> func) {
				form._onDrawCheck = func;
				return *this;
			}
			template <auto &... child>
			constexpr Builder<T>& setChildren() {
				form.childrenUpdateAndCheckChange = []() {
					((child.update()), ...);  //folding expression - call methond (c++17)
					return (false || ... || child.is_changed());
				};
				form.childrenRedraw = []() {
					((child.draw()), ...); //folding expression Draw method (c++17)
				};
				return *this;
			}
		};

	public:

		void draw() {
			bool is_changed = _is_changed;

			if (_onDrawCheck != nullptr) {
				is_changed = is_changed || _onDrawCheck(this);
			}
			bool isChildrenNeedRedraw = childrenUpdateAndCheckChange();

			if (is_changed || isChildrenNeedRedraw) {
				_redraw();
			}
		}
		void visibility(bool is_visible) {
			if (_is_visible == is_visible) { return; }

			_is_visible = is_visible;
			_is_changed = true;
		}
	};



	template <typename DisplayType>
	class TNoBorderForm: public TAbstractForm<DisplayType> {
		using Self = TNoBorderForm<DisplayType>;
		using Parent = TAbstractForm<DisplayType>;
		template <typename> friend struct  Parent::Builder;
		//friend class Parent;
	protected:
		static void _redraw(Parent* form) {
			Self* f = static_cast<Self*>(form);
			if (!f->_is_visible) { return; }
			f->DISPLAY->fill(Parent::DisplayColor::Black());
			f->childrenRedraw();
			f->DISPLAY->RequestForUpdate();
		}

		TNoBorderForm(DisplayType* display): Parent(display, _redraw){}
	public:
		struct Builder : public Parent::template Builder<Self> {
			constexpr Builder(DisplayType* display): Parent::template Builder<Self>(display){
			}
		};
	};

	template <typename DisplayType>
	class TCaptionForm : public TAbstractForm<DisplayType> {
		using Self = TCaptionForm<DisplayType>;
		using Parent = TAbstractForm<DisplayType>;
		template <typename> friend struct  Parent::Builder;
		TTextBlock<DisplayType> _caption;
		static void _redraw(Parent* form){
			Self* f = static_cast<Self*>(form);
			if (!f->_is_visible) { return; }
			enum { CAPTION_HEIGHT = 12, TITLE_LEFT_OFFSET = 2 };
			f->DISPLAY->fill(Parent::DisplayColor::Black());
			f->DISPLAY->drawFilledRectangle(0, 0, f->_size.Width, CAPTION_HEIGHT,Parent::DisplayColor::White());
			f->_caption.update();
			f->_caption.draw();
			f->childrenRedraw();
			f->DISPLAY->RequestForUpdate();
		}
	protected:
		constexpr TCaptionForm(DisplayType* display,TTextBlock<DisplayType> tb): Parent(display,_redraw),_caption(tb) {}
	public:
		struct Builder : public Parent::template Builder<Self> {
			using BaseClass = typename Parent::template Builder<Self>;
			constexpr Builder(DisplayType* display,TTextBlock<DisplayType> tb): BaseClass(display,tb){}
		};
	};

	template <typename DisplayType>
	class TCustomForm : public TAbstractForm<DisplayType> {
		using Parent = TAbstractForm<DisplayType>;
		using Self = TCustomForm<DisplayType>;
	protected:
		f_onEventPtr<DisplayType> _onDraw = []() {};
		static void _redraw(Parent* form) {
			Self* f = static_cast<Self*>(form);
			if (!f->_is_visible) { return; }
			f->DISPLAY->fill(Parent::DisplayColor::Black());
			(*f->_onDraw)(f);
			f->childrenRedraw();
			f->DISPLAY->RequestForUpdate();
		}
		constexpr TCustomForm(DisplayType* display, f_onEventPtr<DisplayType> onDraw) : Parent(display,_redraw), _onDraw(onDraw) {}
	public:
		struct Builder : public Parent::template Builder<Self> {
			constexpr Builder(DisplayType* display, f_onEventPtr<DisplayType> onDraw): Parent(display,_onDraw) {}
		};
	};
	
	
	template <typename DisplayType>
	class TTextBlock {
			using DisplayColor = typename DisplayType::DisplayColor;
			char _currentValue[TB_TEXT_LENGTH] = { 0 };
			DisplayType* DISPLAY;
			tb_getValuePtr<DisplayType> _value_ptr;
			DisplayColor _color = DisplayColor::White();
			TEXT_ALIGNMENT _h_align = ALIGNMENT_LEFT;
			TEXT_VALIGNMENT _v_align = VALIGNMENT_CENTER;
			//TForm* _parent;
			GeometricSize _size = { DisplayType::Width, DisplayType::Height };
			vd::Point2D<uint16_t> _pos = { 0,0 }; //left_bottom;
			bool _is_init = false;
			bool _is_selected = false;
			bool _is_changed = true;
			const font_t* _font;
			vd::Point2D<uint16_t> _alignText(u16 width, u16 height, const font_t* font,
			                    const char* str, TEXT_ALIGNMENT h_align = ALIGNMENT_LEFT,
			                    TEXT_VALIGNMENT v_align = VALIGNMENT_CENTER) {
				vd::Point2D<uint16_t> res;
				font_size_t size = font->getSize(str);

				switch (h_align) {
				case ALIGNMENT_LEFT:
					res.x = 0;
					break;

				case ALIGNMENT_CENTER:
					res.x = u16(width - size.Length) / 2;
					break;

				case ALIGNMENT_RIGHT:
					res.x = u16(width - size.Length - 1);
					break;

				default:
					break;
				}

				switch (v_align) {
				case VALIGNMENT_BOTTOM:
					res.y = u16(height - size.Height);
					break;

				case VALIGNMENT_CENTER:
					res.y = u16((height - size.Height) / 2);
					break;

				case VALIGNMENT_TOP:
					res.y = 0;
					break;
				}

				return res;
			}
			void _redraw() {
				_is_changed = false;
				vd::Point2D p = _alignText(_size.Width, _size.Height, _font,
				                        _currentValue, _h_align, _v_align);
				DisplayColor text_color = _color;
				DisplayColor bg_color = _color.Inverted();

				if (_is_selected) {
					DisplayColor t = text_color;
					text_color = bg_color;
					bg_color = t;
				}

				if (bg_color == DisplayColor::White()) {
					DISPLAY->drawFilledRectangle(_pos.x, _pos.y, _size.Width, _size.Height,
					                            bg_color);
				}

				DISPLAY->gotoXY(_pos.x + p.x, _pos.y + p.y);
				DISPLAY->puts(_currentValue, _font, text_color);
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
			constexpr TTextBlock(DisplayType* display,const font_t& f, tb_getValuePtr<DisplayType> func): DISPLAY(display), _value_ptr(func), _font(&f) {
			}
		public:

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
				if (_font != &f) {
					_font = &f;
					_is_changed = true;
				}
			}
			void setColor(DisplayColor c) {
				if (c == _color)  { return; }

				_color = c;
				_is_changed = true;
			}
			DisplayColor getColor()const {
				return _color;
			}

			struct Builder {
			private:
				TTextBlock<DisplayType> _tb;
			public:
				constexpr Builder(DisplayType* display, const font_t& f, tb_getValuePtr<DisplayType> func): _tb(display,f,func) {}

				constexpr TTextBlock<DisplayType> create() {
					return _tb;
				}
				constexpr Builder& setPosition(const vd::Point2D<uint16_t>& p) {
					_tb._pos = p;
					return *this;
				}
				constexpr Builder& setSize(const  GeometricSize& size) {
					_tb._size = size;
					return *this;
				}
				constexpr Builder& setColor(DisplayColor c) {
					_tb._color = c;
					return *this;
				}
				constexpr Builder& setFont(const font_t& font) {
					_tb._font = font;
					return *this;
				}
				constexpr Builder& setAlign(TEXT_ALIGNMENT ha,TEXT_VALIGNMENT va) {
					_tb._h_align = ha;
					_tb._v_align = va;
					return *this;
				}
			};

	};



	template <typename DisplayType>
	class TIcon {
			using DisplayColor = typename DisplayType::DisplayColor;
			DisplayType* DISPLAY;
			const font_t& _icon;
			bool _is_visible = true;
			bool _is_changed = true;
			vd::Point2D<uint16_t> _pos = { 0,0 };
			DisplayColor _color = DisplayColor::White();
			void _redraw() {
				_is_changed = false;

				if (_is_visible) {
					DISPLAY->drawIcon(_pos.x, _pos.y, &_icon,
					                 _color);
				}
			}
			constexpr TIcon(DisplayType* display,const font_t& icon):DISPLAY(display), _icon(icon) {}
		public:

			//void update();
			[[nodiscard]] bool is_changed() const {
				return _is_changed;
			}
			void draw() {
				_redraw();
			}
			[[nodiscard]] bool is_visible() const {
				return _is_visible;
			}
			void visible(bool is_visible) {
				if (_is_visible != is_visible) {
					_is_visible = is_visible;
					_is_changed = true;
				}
			}
			void update() {

			}
			struct Builder {
			private:
				TIcon<DisplayType> _icon;
			public:

				constexpr Builder(DisplayType* display,const font_t& icon) : _icon(display,icon) {}

				constexpr TIcon<DisplayType> create() {
					return _icon;
				}
				constexpr Builder& setPosition(const vd::Point2D<uint16_t>& p) {
					_icon._pos = p;
					return *this;
				}
				constexpr Builder& setColor(DisplayColor c) {
					_icon._color = c;
					return *this;
				}
			};

	};
}
