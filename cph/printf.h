#pragma once



#ifdef _AVR
#include <avr\pgmspace.h>
#include <stdio.h>
#include <string.h> //for strchr
#define __printf printf_P

namespace cph {

	struct PrintF {
		private:
			PrintF() = delete;
		public:
			template <class OutputFunctor>
			static void init(OutputFunctor&& f) {
				static OutputFunctor& _functor = f;
				static FILE custom_stdout = { 0, 0, _FDEV_SETUP_WRITE, 0, 0, [](char c, FILE * stream)->int {
						_functor(c);
						return 1; }, NULL, 0
				};
				stdout = &custom_stdout;
			}

	};

}

#elif defined (_ARM)
#include <stdio.h>
#include "platform/stm32/core/platform_itm.h"
namespace cph {

struct PrintF {
	private:
		typedef void(*printf_func_t)(char c);
		PrintF() = delete;
	public:
		static printf_func_t printf_func;
		template <class OutputFunctor>
		static void init(OutputFunctor&& f) {
			static const OutputFunctor& _functor = f;
			printf_func = [](char c){ _functor(c);};
		}
};

}

#elif defined (_WIN32)
#include <stdio.h>
#include <string.h> //for strchr
#define PSTR(x) x
#define __printf printf
#endif

#ifdef __INTELLISENSE__
	#ifdef PSTR
		#undef PSTR
	#endif
	#define PSTR(x) x
#endif



#define DEBUG_MACRO

#ifdef DEBUG_MACRO
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__) //variable- short filename

//#define dprintf __printf(PSTR("%s:"),__FILENAME__);__printf
#define dprintf __printf
#else
inline void dprintf(const char* __fmt, ...) {}
inline void dprintf_s(const char* __fmt, ...) {}
#endif


