#pragma once
#ifdef _AVR

	#include "platform/avr/core/platform_exti.h"
#elif defined(_ARM)
#include "platform/stm32/core/platform_exti.h"
#endif
#pragma once
