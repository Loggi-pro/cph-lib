#pragma once
#include "cph-library/cph/gpio.h"
#include "void/void/static_assert.h"
#include "cph-library/cph/basic_usart.h"
#include "cph-library/cph/iostream.h"
#include "cph-library/cph/system_counter.h"
#include "cph-library/cph/gpio.h"

typedef cph::basic_ostream<cph::AdapterUsartSync_Debug<cph::Usart0>> ostream;
typedef cph::TTimerConfig<cph::TIMER_0, cph::TimerPrecision::Resolution_1ms> TIMER_CONFIG;
extern ostream cout;
typedef cph::io::Pb5 pinLed;