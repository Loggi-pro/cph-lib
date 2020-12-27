#pragma once
#ifdef _ARM
	#include "platform/stm32/i2c/platform_i2c.h"
#endif
namespace cph {

	enum I2C_NUMBER {
		I2C_FAKE = -1
		           #ifdef CPH_HAS_I2C_1
		, I2C_1 = 1
		           #endif
		#ifdef CPH_HAS_I2C_2
		, I2C_2 = 2
		#endif
	};


	template <I2C_NUMBER N>
	struct  i2c_t {
		private:
			typedef cph::Private::i2c_hardware_t<N> Hardware;

		public:

			static void init(uint32_t address) {
				Hardware::init(address);
			}
			static void enable() {
				Hardware::enable();
			}
			static void disable() {
				Hardware::disable();
			}
			static bool writeData(uint16_t address,const vd::gsl::span<uint8_t>& data) {
				return Hardware::writeData(address,data);
			}
            static bool writeData(uint16_t address,uint16_t memAddress,const vd::gsl::span<uint8_t>& data) {
				return Hardware::writeData(address,memAddress,data);
			}
			static bool writeByte(uint16_t address,uint8_t byte) {
				return Hardware::writeByte(address,byte);
			}


            		static bool writeByte(uint16_t address,uint16_t memAddress,uint8_t byte) {
				return Hardware::writeByte(address,memAddress,byte);
			}

			static void reset() {
				Hardware::reset();
			}

	};



    template <I2C_NUMBER N,uint32_t ADDRESS>
    struct i2c_Binded_t:public i2c_t<N> {
        		static void writeData(vd::gsl::span<uint8_t>& data) {
				i2c_t<N>::Hardware::writeData(ADDRESS,data);
			}
			static void writeByte(uint8_t byte) {
				i2c_t<N>::Hardware::writeByte(ADDRESS,byte);
			}
    };
}