#pragma once
namespace cph {
class Random {
		static inline uint32_t _seed = 0;
		static inline uint32_t _rnd_math(uint32_t
		                                 x) {  // a good random number generator; call with 1 <= x <=M-1
			x = (x >> 16) + ((x << 15) & 0x7FFFFFFF) - (x >> 21) - ((x << 10) & 0x7FFFFFFF);

			if ((int32_t)x < 0) { x += 0x7FFFFFFF; }

			//if (x == 0) { return 1; }
			return x;
		};



		static uint32_t _rand2() {
			uint32_t value = _rnd_math(_seed);
			_seed ^= value;
			return value;
		}
		template <typename T>
		inline static T limit(T x, T min, T max) {
			return x % (max - min) + min;
		}
	public:
		static void init(uint32_t seed) { _seed = seed; }
		template < typename T>
		static T get(T min, T max);
		template <>
		static uint32_t get<uint32_t>(uint32_t min, uint32_t max) {
			uint32_t value = _rnd_math(_seed);
			_seed ^= value;
			return limit(value, min, max);
		}
		//much simpler generator, but dont work good with big numbers
		template <>
		static uint16_t get<uint16_t>(uint16_t min, uint16_t max) {
			_seed = (3661 * _seed + 30809);
			return limit((uint16_t)_seed, min, max);
		}
		template < typename T>
		static T get(T max) {
			return get<T>(0, max);
		}
};

}