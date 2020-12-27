#include "vemacro.h"
#include "display/ssd1306.h"
#include <cassert>
#include <SFML/Graphics.hpp>
#include "bit_field.h"


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



class TGraphicWindow {


		sf::RenderWindow _window;
		sf::Texture _texture;
		sf::Sprite* _sprite;

	public:
		TGraphicWindow(const char title[], const int Width, const int Height,
		               const int Scale) : _window(sf::VideoMode(Width * Scale,
			                       Height * Scale), title),	_texture() {
			_window.setFramerateLimit(25);
			_window.setActive(false);
			_texture.create(Width, Height);
			_sprite = new sf::Sprite(_texture);
			_sprite->setScale((float)Scale, (float)Scale);
		}
		~TGraphicWindow() {
			_window.close();
			delete _sprite;
		}

		sf::Texture& getTexture() {
			return _texture;
		}

		void draw(sf::Uint8& bitmap) {
			if (!_window.isOpen()) { return; }

			sf::Event event;

			while (_window.pollEvent(event)) {
				// "close requested" event: we close the window
				if (event.type == sf::Event::Closed) {
					_window.close();
				}
			}

			_texture.update(&bitmap);
			//draw here
			_window.draw(*_sprite);
			_window.display();
		}
};

TGraphicWindow* Fbitmap = nullptr;

#define MIN(n,m) (((n) < (m)) ? (n) : (m))
#define MAX(n,m) (((n) < (m)) ? (m) : (n))

const int SCALE = 800 / (MAX(SSD1306_WIDTH, SSD1306_HEIGHT));









void SSD1306_Init(void) {
	assert(Fbitmap == nullptr); //"SSD1306_Init used several times"
	Fbitmap = new TGraphicWindow("SSD1306", SSD1306_WIDTH, SSD1306_HEIGHT, SCALE);
}


const int PAGE_HEIGHT = MIN(SSD1306_HEIGHT, 8);
const int PAGE_WIDTH = SSD1306_WIDTH;

/* {SSD1306 buffer indexation}
<PAGE0> <INDEXATION>
 0  8 16 24
 1  9 17 25
 2 10 18 26
 3 11 19 27
 4 12 20 28
 5 13 21 29
 6 14 22 30
 7 15 23 31
<PAGE1>
32 40 48 56
33 41 49 57
...	      ...
*/


/*{standart indexation}
<PAGE0> < INDEXATION>
 0  1  2  3
 4  5  6  7
 8  9 10 11
12 13 14 15
16 17 18 19
20 21 22 23
24 25 26 27
28 29 30 31
< PAGE1 >
...
*/

sf::Uint8& convert_bitmap_from_page_to_conseq(const Tbit_array*
        SSD1306_buffer) {
	//convert SSD1306 buffer to standart bitmap
	static sf::Uint8 pixels[SSD1306_WIDTH  * SSD1306_HEIGHT * sizeof(TColor32bit)];
	TColor32bit color;
	TColor32bit* pix = (TColor32bit*)pixels;

	for (int i = 0; i < SSD1306_WIDTH * SSD1306_HEIGHT; i++) {
		color = bf_bit_is_set(SSD1306_buffer, i) ? ColorGreen : ColorBlack;
		int inside_page = i % (PAGE_WIDTH * PAGE_HEIGHT);
		int x = inside_page / PAGE_HEIGHT;
		int y = inside_page % PAGE_HEIGHT;
		int page = i / (PAGE_WIDTH * PAGE_HEIGHT);
		int d = x + y * PAGE_WIDTH + page * PAGE_WIDTH * PAGE_HEIGHT;
		pix[d] = color;
	}

	return *pixels;
}

void SSD1306_UpdateScreen(const u08* buffer) {
	assert(Fbitmap != nullptr); //"SSD1306 not inited"
	Fbitmap->draw(convert_bitmap_from_page_to_conseq(buffer));
}


void SSD1306_ON(void) {
}
void SSD1306_OFF(void) {
}

u16 SSD1306_Get1DIndex(u16 x, u16 y) {
	u08 n_bit = y % 8;
	u16 n_byte = (x + (y / 8) * SSD1306_WIDTH);
	return  n_byte * 8 + /*bit in byte*/ n_bit;
}


void SSD1306_HardReset(void){

}

#undef MAX
#undef MIN