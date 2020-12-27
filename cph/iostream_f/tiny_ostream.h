#pragma once

#include <cph/iostream_f/tiny_ios.h>
#include <cph/enum.h>
#include <cph/string.h>
#include <void/limits.h>
#include <void/math.h> //isSigned,isNegative
//#include <cph/utils/template_math.h>

namespace cph {

	template<class OutputPolicy, class char_type = char, class IOS = basic_ios<char_type>>
	class basic_ostream : public OutputPolicy, public IOS {
		typedef basic_ostream Self;

	private:
		inline uint8_t Base();

		inline void FieldFill(streamsize_t lastOutputLength, typename IOS::fmtflags mask);

		template<class T>
		inline void PutInteger(T value);

		inline void PutFloat(float value);

		inline void PutBool(bool value);

	public:
		typedef typename IOS::trates trates;

		constexpr basic_ostream() = default;

		Self &operator<<(bool value) {
			PutBool(value);
			return *this;
		}

		Self &operator<<(uint8_t value) {
			PutInteger(value);
			return *this;
		}

		Self &operator<<(int8_t value) {
			PutInteger(value);
			return *this;
		}

		Self &operator<<(uint16_t value) {
			PutInteger(value);
			return *this;
		}

		Self &operator<<(int16_t value) {
			PutInteger(value);
			return *this;
		}

		Self &operator<<(uint32_t value) {
			PutInteger(value);
			return *this;
		}

		Self &operator<<(int32_t value) {
			PutInteger(value);
			return *this;
		}

		Self &operator<<(float value) {
			PutFloat(value);
			return *this;
		}

		Self &operator<<(const char_type *value) {
			puts(value);
			return *this;
		}

		Self &operator<<(char_type value) {
			OutputPolicy::operator()(value);
			return *this;
		}

		Self &
		operator<<(Self &(*__pf)(Self &)) {
			return __pf(*this);
		}

		Self &
		operator<<(ios_base &(*__pf)(ios_base &)) {
			__pf(*this);
			return *this;
		}

		void puts(const char_type *str) {
			const size_t outputSize = trates::StrLen(str);
			FieldFill(outputSize, IOS::right);
			write(str, outputSize);
			FieldFill(outputSize, IOS::left);
		}

		template<class CharPtr>
		void puts(CharPtr str) {
			CharPtr strEnd = str;

			while (*strEnd) {
				++strEnd;
			}

			streamsize_t outputSize = strEnd - str;
			FieldFill(outputSize, IOS::right);

			while (char_type c = *str) {
				OutputPolicy::operator()(c);
				++str;
			}

			FieldFill(outputSize, IOS::left);
		}

		template<class CharPtr>
		void write(CharPtr str, size_t size) {
			CharPtr lend = str + size;
			write(str, lend);
		}

		template<class CharPtr>
		void write(CharPtr abegin, CharPtr aend) {
			while (abegin != aend) {
				OutputPolicy::operator()(*abegin);
				++abegin;
			}
		}

		void put(char c) const {
			OutputPolicy::operator()(c);
		}
	};

	template<class OutputPolicy, class char_type, class IOS>
	inline basic_ostream<OutputPolicy, char_type, IOS> &endl(
			basic_ostream<OutputPolicy, char_type, IOS> &os) {
		os.put('\n');
		return os;
	}

	template<class OutputPolicy, class char_type, class IOS>
	inline basic_ostream<OutputPolicy, char_type, IOS> &ends(basic_ostream<OutputPolicy, char_type, IOS> &os) {
		os.put('\0');
		return os;
	}

	template<class OutputPolicy, class char_type, class IOS>
	uint8_t basic_ostream<OutputPolicy, char_type, IOS>::Base() {
		if (IOS::flags() & IOS::hex) {
			return 16;
		}

		if (IOS::flags() & IOS::oct) {
			return 8;
		}

		if (IOS::flags() & IOS::bin) {
			return 2;
		}

		return 10;
	}

	template<class OutputPolicy, class char_type, class IOS>
	void basic_ostream<OutputPolicy, char_type, IOS>::FieldFill(streamsize_t lastOutputLength,
																typename IOS::fmtflags mask) {
		if (IOS::flags() & mask) {
			streamsize_t width = IOS::width(0);

			if (width < lastOutputLength) {
				return;
			}

			streamsize_t fillcount = width - lastOutputLength;
			char_type c = IOS::fill(' ');

			for (streamsize_t i = 0; i < fillcount; i++) {
				OutputPolicy::operator()(c);
			}
		}
	}

	template<class OutputPolicy, class char_type, class IOS>
	template<class T>
	void basic_ostream<OutputPolicy, char_type, IOS>::PutInteger(T value) {
		const int bufferSize = cph::ConvertBufferSize<T>::value;
		char_type buffer[bufferSize + 1];
		const int maxPrefixSize = 3;
		char_type prefix[maxPrefixSize];
		char_type *prefixPtr = prefix + maxPrefixSize;

		if ((IOS::flags() & (IOS::hex | IOS::oct)) == 0) {
			if (vd::numeric_limits<T>::is_signed) {
				if (vd::IsNegative(value)) {
					value = vd::Abs(value);
					*--prefixPtr = trates::Minus();
				} else if (IOS::flags() & IOS::showpos) {
					*--prefixPtr = trates::Plus();
				}
			} else {
				if (IOS::flags() & IOS::showpos) {
					*--prefixPtr = trates::Plus();
				}
			}
		} else if (IOS::flags() & IOS::showbase) {
			if (IOS::flags() & IOS::hex) {
				*--prefixPtr = 'x';
			}

			*--prefixPtr = '0';
		}

		typedef typename vd::Unsigned<T>::Result UT;
		UT uvalue = static_cast<UT>(value);
		char_type *str = cph::Utoa(uvalue, buffer + bufferSize, Base());
		size_t outputSize = size_t(buffer + bufferSize - str + prefix + maxPrefixSize - prefixPtr);
		FieldFill(outputSize, IOS::right);
		write(prefixPtr, prefix + maxPrefixSize);
		FieldFill(outputSize, IOS::internal);
		write(str, buffer + bufferSize);
		FieldFill(outputSize, IOS::left);
	}

	template<class OutputPolicy, class char_type, class IOS>
	void basic_ostream<OutputPolicy, char_type, IOS>::PutBool(bool value) {
		if (IOS::flags() & IOS::boolalpha) {
			if (value) {
				puts(trates::True());
			} else {
				puts(trates::False());
			}
		} else {
			FieldFill(1, IOS::right);

			if (value) {
				OutputPolicy::operator()(trates::DigitToLit(1));
			} else {
				OutputPolicy::operator()(trates::DigitToLit(0));
			}

			FieldFill(1, IOS::left);
		}
	}

	template<class OutputPolicy, class char_type, class IOS>
	void basic_ostream<OutputPolicy, char_type, IOS>::PutFloat(float value) {
		const int bufferSize = 10;
		char_type buffer[bufferSize + 1];
		streamsize_t precision = IOS::precision();

		if (precision > bufferSize - 1) {
			precision = bufferSize - 1;
		}

		if (precision <= 2) {
			precision = 2;
		}

		int exp10 = ftoaEngine(value, buffer, precision);

		if (exp10 == 0xff) {
			char_type *ptr = buffer;

			if (buffer[0] == cph::char_traits<char_type>::Plus() && (IOS::flags() & IOS::showpos) == 0) {
				ptr++;
			}

			puts(ptr);
			return;
		}

		char_type *str_begin = &buffer[2];

		if (buffer[1] != cph::char_traits<char_type>::DigitToLit(0)) {
			exp10++;
			str_begin--;
		}

		uint_fast8_t digits = cph::char_traits<char_type>::StrLen(str_begin);
		uint_fast8_t intDigits = 0, leadingZeros = 0;

		if ((streamsize_t) vd::Abs<int>(exp10) >= precision) {
			intDigits = 1;
		} else if (exp10 >= 0) {
			intDigits = exp10 + 1;
			exp10 = 0;
		} else {
			intDigits = 0;
			leadingZeros = -exp10 - 1;
			exp10 = 0;
		}

		uint_fast8_t fractDigits = digits > intDigits ? digits - intDigits : 0;

		if (buffer[0] == cph::char_traits<char_type>::Minus() || (IOS::flags() & IOS::showpos)) {
			OutputPolicy::operator()(buffer[0]);
		}

		if (intDigits) {
			uint_fast8_t count = intDigits > digits ? digits : intDigits;

			while (count--) {
				OutputPolicy::operator()(*str_begin++);
			}

			int_fast8_t tralingZeros = intDigits - digits;

			while (tralingZeros-- > 0) {
				OutputPolicy::operator()(cph::char_traits<char_type>::DigitToLit(0));
			}
		} else {
			OutputPolicy::operator()(cph::char_traits<char_type>::DigitToLit(0));
		}

		if (fractDigits) {
			OutputPolicy::operator()(cph::char_traits<char_type>::DecimalDot());

			while (leadingZeros--) {
				OutputPolicy::operator()(cph::char_traits<char_type>::DigitToLit(0));
			}

			while (fractDigits--) {
				OutputPolicy::operator()(*str_begin++);
			}
		}

		if (exp10 != 0) {
			OutputPolicy::operator()(cph::char_traits<char_type>::Exp());
			uint_fast8_t upow10;

			if (exp10 < 0) {
				OutputPolicy::operator()(cph::char_traits<char_type>::Minus());
				upow10 = -exp10;
			} else {
				OutputPolicy::operator()(cph::char_traits<char_type>::Plus());
				upow10 = exp10;
			}

			char *powPtr = cph::UtoaFastDiv(upow10, buffer + bufferSize);

			while (powPtr < buffer + bufferSize) {
				OutputPolicy::operator()(*powPtr++);
			}
		}
	}


	template<class OutputPolicy, class char_type = char, class IOS = basic_ios<char_type>>
	class null_ostream : basic_ostream<OutputPolicy, char_type, IOS> {
		typedef null_ostream Self;
	public:

		Self &operator<<(bool value) {
			return *this;
		}

		Self &operator<<(uint8_t value) {
			return *this;
		}

		Self &operator<<(int8_t value) {
			return *this;
		}

		Self &operator<<(uint16_t value) {
			return *this;
		}

		Self &operator<<(int16_t value) {
			return *this;
		}

		Self &operator<<(uint32_t value) {
			return *this;
		}

		Self &operator<<(int32_t value) {
			return *this;
		}

		Self &operator<<(float value) {
			return *this;
		}

		Self &operator<<(const char_type *value) {
			return *this;
		}

		Self &operator<<(char_type value) {
			return *this;
		}

		Self &
		operator<<(Self &(*__pf)(Self &)) {
			return __pf(*this);
		}

		Self &
		operator<<(ios_base &(*__pf)(ios_base &)) {
			return *this;
		}
	};


	template<class OutputPolicy, class char_type, class IOS>
	inline null_ostream<OutputPolicy, char_type, IOS> &endl(
			null_ostream<OutputPolicy, char_type, IOS> &os) {
		return os;
	}

	template<class OutputPolicy, class char_type, class IOS>
	inline null_ostream<OutputPolicy, char_type, IOS> &ends(
			null_ostream<OutputPolicy, char_type, IOS> &os) {
		return os;
	}


}