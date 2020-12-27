#pragma once

#include <void/math.h>
#include "char_traits.h"
namespace cph {
	template<class T, class char_type>
	char_type* UtoaFastDiv(T value, char_type* bufferEnd) {
		*bufferEnd = 0;
		vd::divmod10_t<T> res;
		res.quot = value;

		do {
			res = vd::divmodu10(res.quot);
			*--bufferEnd = char_traits<char_type>::DigitToLit(res.rem);
		} while (res.quot);

		return bufferEnd;
	}

	template<class T, class char_type>
	char_type* UtoaOct(T value, char_type* bufferEnd) {
		*bufferEnd = 0;

		do {
			uint8_t c = value & 0x07;
			value >>= 3;
			*--bufferEnd = char_traits<char_type>::DigitToLit(c);
		} while (value);

		return bufferEnd;
	}

	template<class T, class char_type>
	char_type* UtoaHex(T value, char_type* bufferEnd) {
		*bufferEnd = 0;
		uint8_t count = 0;

		do {
			uint8_t c = value & 0x0f;
			value >>= 4;
			count ++;
			* --bufferEnd = char_traits<char_type>::DigitToLit(c);
		} while (value);

		//padding

		while (count < sizeof(T) * 2) {
			*--bufferEnd = char_traits<char_type>::DigitToLit(0);
			count++;
		}

		return bufferEnd;
	}

	template<class T, class char_type>
	char_type* UtoaBin(T value, char_type* bufferEnd) {
		*bufferEnd = 0;
		uint8_t count = 0;

		do {
			uint8_t c = value & 0x01;
			value >>= 1;
			count++;
			*--bufferEnd = char_traits<char_type>::DigitToLit(c);
		} while (value);

		//padding
		while (count < sizeof(T) * 8) {
			*--bufferEnd = char_traits<char_type>::DigitToLit(0);
			count++;
		}

		{ return bufferEnd; }
	}

	template<class T, class char_type>
	char_type* UtoaBuiltinDiv(T value, char_type* bufferEnd, uint8_t radix = 10) {
		char_type* ptr = bufferEnd;

		do {
			T q = value / radix;
			T rem = value - q * radix;
			value = q;
			*--ptr = char_traits<char_type>::DigitToLit(rem);
		} while (value != 0);

		return ptr;
	}

	template<class T, class char_type>
	char_type* Utoa(T value, char_type* bufferEnd, uint8_t radix = 10) {
		if (radix == 10)
		{ return UtoaFastDiv(value, bufferEnd); }

		if (radix == 16)
		{ return UtoaHex(value, bufferEnd); }

		if (radix == 8)
		{ return UtoaOct(value, bufferEnd); }

		if (radix == 2)
		{ return UtoaBin(value, bufferEnd); }

		*bufferEnd = 0;
		return bufferEnd;
	}

	template<class T>
	struct ConvertBufferSize {
		static const int value = sizeof(T) * 8 + 1;
	};

	template<class char_type>
	inline bool isdigit(char_type c)	{ return c >= '0' && c <= '9';}

	template<class char_type>
	inline bool isoctdigit(char_type c)	{ return c >= '0' && c <= '7';}

	template<class char_type>
	inline bool isxdigit(char_type c) {
		return isdigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
	}

	template<class char_type>
	inline bool isspace(char_type c) {
		return c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\0';
	}

	// ASCII chars only
	template<class char_type>
	inline char_type tolower(char_type c) {
		if (c >= 'A' && c <= 'Z')
		{ return c - 'A' + 'a'; }

		return c;
	}

	template<class char_type>
	inline bool toapper(char_type c) {
		if (c >= 'a' && c <= 'z')
		{ return c - 'a' + 'A'; }

		return c;
	}

	template<class Str1T, class Str2T>
	inline bool StringMatch(Str1T str1, Str2T str2) {
		do {
			if (*str2 == 0)
			{ return true; }
		} while (*str1++ == *str2++);

		return *str1 == *str2;
	}

	template<class T, class StrT>
	T StringToIntDec(StrT& str) {
		T result = 0;
		uint_fast8_t count = ConvertBufferSize<T>::value;

		while (isdigit(*str) && --count) {
			result = result * 10 + (*str - '0');
			str++;
		}

		return result;
	}

	template<class T, class StrT>
	T StringToIntOct(StrT& str) {
		T result = 0;
		uint_fast8_t count = ConvertBufferSize<T>::value;

		while (isoctdigit(*str) && --count) {
			result = result * 8 + (*str - '0');
			str++;
		}

		return result;
	}

	template<class T, class StrT>
	T StringToIntHex(StrT& str) {
		T result = 0;
		int delta;
		uint_fast8_t count = ConvertBufferSize<T>::value;

		while (--count) {
			if (isdigit(*str))
			{ delta = '0'; }
			else if ((*str >= 'A' && *str <= 'F'))
			{ delta = 'A' - 10; }
			else if ((*str >= 'a' && *str <= 'f'))
			{ delta = 'a' - 10; }
			else { break; }

			result = result * 16 + (*str - delta);
			str++;
		}

		return result;
	}

	template<class T>
	inline uint8_t DecimalDigits (T x) {
		T p = 1;
		uint8_t i;
		const uint8_t max = sizeof(x) == 2 ? 5 : sizeof(x) == 4 ? 10 : sizeof(x) == 8 ? 20 : 3;

		for (i = 0; i <= max; i++) {
			if (x < p) {
				return i;
			}

			p *= 10;
		}

		return i;
	}
}
