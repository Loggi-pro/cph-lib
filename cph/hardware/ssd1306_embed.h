#include <void/delay.h>
#include <void/gsl.h>
#include <cph/gui/graphic_buffer.h>
#include <cph/gpio.h>
#include <cph/async.h>
#include <cph/timer.h>
namespace cph {
	template <class I2C, uint8_t SSD1306_I2C_ADDR = 0x78,/*0x7A  or  0x78*/uint32_t SSD_WIDTH = 128, uint32_t SSD_HEIGHT = 64, typename enablePin = cph::io::NullPin>
	struct ssd1306_t: public graphic_buffer_t<BufferTwoColored<SSD_WIDTH, SSD_HEIGHT>, SSD1306_Color> {
		public:
			static constexpr inline uint32_t Width = SSD_WIDTH;
			static constexpr inline uint32_t Height = SSD_HEIGHT;
			using DisplayColor = SSD1306_Color;
		private:
			typedef graphic_buffer_t< BufferTwoColored<SSD_WIDTH, SSD_HEIGHT>, SSD1306_Color> TGraphicBuffer;
			bool _isInited;
			uint8_t _state = 0;
			uint8_t _isNeedUpdate;
			bool _isInUpdateMode;
			cph::timer_t _timer;

			bool _writeCommand(uint8_t command) {
				bool res = I2C::writeByte(SSD1306_I2C_ADDR, 0x00, command);
				//assert(res);
				return res;
				//return true;
			}
			bool _writeData(const vd::gsl::span<uint8_t>& data) {
				bool res = I2C::writeData(SSD1306_I2C_ADDR, 0x40, data);
				//assert(res);
				return res;
				//return true;
			}
		public:

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
			ssd1306_t(): _isInited(false), _state(0), _isNeedUpdate(0) {}
			/**
			 * @brief  Initializes SSD1306 LCD
			 * @param  None
			 * @retval Initialization status:
			 *           - 0: LCD was not detected on I2C port
			 *           - > 0: LCD initialized OK and ready to use
			 */

			cph::Result<void> init() {
#define STATE(y,x) case y: if (x) {_state++;}; break;

				if (_isInited) { return { true }; }

				switch (_state) {
				case 0:
					if (hardReset().isSuccess()) {_state++;}

					break;
					STATE(1, _writeCommand(0xAE)); //--turn off on SSD1306 panel
					STATE(2, _writeCommand(0x20)); //Set Memory Addressing Mode
					STATE(3, _writeCommand(
					          0x10)); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
					STATE(4, _writeCommand(0xB0)); //Set Page Start Address for Page Addressing Mode,0-7
					STATE(5, _writeCommand(0xC8)); //Set COM Output Scan Direction
					STATE(6, _writeCommand(0x00)); //---set low column address
					STATE(7, _writeCommand(0x10)); //---set high column address
					STATE(8, _writeCommand(0x40)); //--set start line address
					STATE(9, _writeCommand(0x81)); //--set contrast control register
					STATE(10, _writeCommand(0xFF));
					STATE(11, _writeCommand(0xA1)); //--set segment re-map 0 to 127
					STATE(12, _writeCommand(0xA6)); //--set normal display
					STATE(13, _writeCommand(0xA8)); //--set multiplex ratio(1 to 64)
					STATE(14, _writeCommand(0x3F)); //
					STATE(15, _writeCommand(0xA4)); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
					STATE(16, _writeCommand(0xD3)); //-set display offset
					STATE(17, _writeCommand(0x00)); //-not offset
					STATE(18, _writeCommand(0xD5)); //--set display clock divide ratio/oscillator frequency
					STATE(19, _writeCommand(0xF0)); //--set divide ratio
					STATE(20, _writeCommand(0xD9)); //--set pre-charge period
					STATE(21, _writeCommand(0x22)); //
					STATE(22, _writeCommand(0xDA)); //--set com pins hardware configuration
					STATE(23, _writeCommand(0x12));
					STATE(24, _writeCommand(0xDB)); //--set vcomh
					STATE(25, _writeCommand(0x20)); //0x20,0.77xVcc
					STATE(26, _writeCommand(0x8D)); //--set DC-DC enable
					STATE(27, _writeCommand(0x14)); //

				case 28:
					TGraphicBuffer::clear();
					_state++;
					break;

				case 29:
					if (updateScreen().isSuccess()) {
						_state++;
					}

					break;
					STATE(30, _writeCommand(0xAF)); //--turn on SSD1306 panel
					STATE(31, _writeCommand(0x14));

				case 32:
					_isInited = true;
					return { true };
					break;
				}

				/* Init I2C */
				//	I2C::init()
				//TODO check I2C is inited
				return {};
#undef STATE
			}
			/**
			 * @brief  Updates buffer from internal RAM to LCD
			 * @note   This function must be called each time you do some changes to LCD, to update buffer from RAM to LCD
			 * @param  None
			 * @retval None
			 */


			cph::Result<void> updateScreen() {
				const u08* buffer = TGraphicBuffer::getBuffer();
				static uint8_t state = 0;
				static uint8_t m  = 0;

				// _writeData(vd::gsl::span<uint8_t>((uint8_t*)buffer, sizeof(buffer)));
				if (m == 8) {
					m = 0;

					if (_isNeedUpdate > 0) {_isNeedUpdate--;}

					if (_isNeedUpdate == 0) {
						_isInUpdateMode = false;
						return { true };
					}
				}

				switch (state) {
				case 0:
					_isInUpdateMode = true;

					if (_writeCommand((uint8_t)(0xB0 + m))) {
						state++;
					}

					break;

				case 1:
					if (_writeCommand((uint8_t)(0x00))) {
						state++;
					}

					break;

				case 2:
					if (_writeCommand(0x10)) {
						state++;
					}

					break;

				case 3:
					if (_writeData(vd::gsl::span<uint8_t>((uint8_t*)(&(buffer[Width * m])), Width))) {
						m++;
						state = 0;
					}

					break;

				default:
					state = 0;
				}

				return {};
			}

			void on() {
				enablePin::Set();
				_writeCommand(0x8D);
				_writeCommand(0x14);
				_writeCommand(0xAF);
			}
			void off () {
				_writeCommand(0x8D);
				_writeCommand(0x10);
				_writeCommand(0xAE);
				enablePin::Clear();
			}
			// Implements reset procedure, toggles reset pin, uses dump delay
			cph::Result<void> hardReset() {
				static uint8_t state = 0;

				switch (state) {
				case 0:
					enablePin::Clear();
					_timer.start();
					state++;
					break;

				case 1:
					if (_timer.isElapsed(10_ms)) {
						enablePin::Set();
						state++;
						_timer.start();
					}

					break;

				case 2:
					if (_timer.isElapsed(10_ms)) { //wait for power
						state = 0;
						return { true };
					}

					break;

				default:
					state = 0;
				}

				return {};
			}
	};

}