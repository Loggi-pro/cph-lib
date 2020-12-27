#pragma once

//uint8_t array to bit array converter

class bit_array_t {
	public:
		struct Proxy {
			uint8_t* byte;
			uint8_t mask;
			Proxy& operator=(bool flag) {
				if (flag) {
					(*byte) |= mask;
				} else {
					(*byte) &= (uint8_t)~mask;
				}

				return *this;
			}
		};
		uint8_t* const data;
		//read
		bool operator[](uint16_t n)const {
			uint16_t packedByteNum = n / 8;
			uint8_t packedBitNum = n % 8;
			bool bit = (data[packedByteNum] >> packedBitNum) & 0x01;
			return bit;
		}
		//write
		Proxy operator[](uint16_t n) {
			uint16_t packedByteNum = n / 8;
			uint8_t packedBitNum = n % 8;
			uint8_t mask = (uint8_t)(1 << packedBitNum);
			return { data + packedByteNum, mask };
		}

		void clear_bits(uint16_t n) {
			uint16_t packedByteNum = n / 8;

			for (uint16_t i = 0; i < packedByteNum; i++) {
				data[i] = 0;
			}

			uint8_t packedBitNum = n % 8;

			//clear all  non 8 aligned bits
			if (packedBitNum != 0) { data[packedByteNum] = 0; }
		}
};