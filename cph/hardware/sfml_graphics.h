#pragma once

#include <SFML/Graphics.hpp>


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

		void draw(uint8_t* bitmap) {
			if (!_window.isOpen()) { return; }

			sf::Event event;

			while (_window.pollEvent(event)) {
				// "close requested" event: we close the window
				if (event.type == sf::Event::Closed) {
					_window.close();
				}
			}

			_texture.update(bitmap);
			//draw here
			_window.draw(*_sprite);
			_window.display();
		}
};