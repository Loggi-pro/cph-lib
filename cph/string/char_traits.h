#pragma once

namespace cph
{
	template<class char_type>
	class char_traits;

	template<>
	class char_traits<char>
	{
		public:
		static char DigitToLit(unsigned digit)
		{
			return static_cast<char>((digit < 10 ? '0' : 'a' - 10) + digit);
		}

		static char Minus()
		{
			return '-';
		}
		static char Plus()
		{
			return '+';
		}

		static char DecimalDot()
		{
			return '.';
		}
		
		static char Exp()
		{
			return 'e';
		}
		static size_t StrLen(const char *str)
		{
			if (*str==0) return 0;
			const char *ptr = str;
			while(*ptr++) continue;
			return (size_t)(ptr - str - 1);
		}

		static const char * False(){return "false";}
		static const char * True(){return "true";}

		typedef int int_type;
		static const int_type eof = -1;
		static const char Null = 0;
	};

	template<>
	class char_traits<wchar_t>
	{
		public:
		static wchar_t DigitToLit(unsigned digit)
		{
			return (wchar_t)((digit < 10 ? L'0' : L'a' - 10) + digit);
		}

		static wchar_t Minus()
		{
			return L'-';
		}

		static wchar_t Plus()
		{
			return L'+';
		}
		static wchar_t DecimalDot()
		{
			return L'.';
		}
		static char Exp()
		{
			return L'e';
		}
		static size_t StrLen(const wchar_t *str)
		{
			if (*str==0) return 0;
			const wchar_t *ptr = str;
			while(*ptr++);
			return (size_t)(ptr - str - 1);
		}

		static const wchar_t * False(){return L"false";}
		static const wchar_t * True(){return L"true";}

		typedef int int_type;
		static const int_type eof = -1;
		static const wchar_t Null = 0;
	};
}
