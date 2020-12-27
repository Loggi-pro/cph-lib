#pragma once
#include <cph/iostream_f/tiny_istream.h>
#include <cph/iostream_f/tiny_ostream.h>
#ifdef _ARM
	#include "platform/stm32/core/platform_itm.h"
#endif

/*
typedef cph::basic_ostream<cph::ITM_Output> ostream;
*/