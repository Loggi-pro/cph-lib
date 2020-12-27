## Overview

List of components:

| COMPONENTS | AVR | STM8 |STM32 (M3/M4) | WIN32 | 
| :----------- | :------: | :------------: | :-----------: |  :-----------: | 
| UART | UART0,UART1 | - | UART1,UART3,UART4 | x|
|TIMER| TIMER0,TIMER1 | x |TIMER_CORE,TIMER1,TIMER_RTC| TIMER_CORE|
| SPI | + |  | + | x|
| ADC | + | x | ADC1 | x|


## Quick Start

1. Use cph headers free.
2. Pass `CPH_LIB_PATH` to cmake (path to cph folder)
2. Some components needs sources and interrupt handlers. See `FindCPH` and `cph-${platform}` cmake files for details.  

## Dependancy
1. `void` library

## Examples
### AVR
See [*examples_avr*](/examples_avr/) folder:
1. [*template_avr*](/examples_avr/template_avr/template_avr.cpp) - template for new project.
5. [*system_timer*](/examples_avr/system_timer/system_timer.cpp) - system timer with different tick periods.
2. [*uart_async*](/examples_avr/uart_async/uart_async.cpp) - asynchronous (interrupt) uart with buffer.
3. [*spi_master*](/examples_avr/spi_master/spi_master.cpp) - blocking spi master (hardware or software).
4. [*spi_slave*](/examples_avr/spi_slave/spi_slave.cpp) - blocking spi master (hardware or software).
5. [*pwm*](/examples_avr/pwm/pwm.cpp) - pwm example with duty controlled by adc potentiometer.
5. [*modbus*](/examples_avr/modbus/modbus.cpp) - modbus example.

## Description
 
### GPIO
* _Header_: [*cph/gpio.h*](/cph/gpio.h) 
* _Require_: no components
* _Description_: 
    1. Control single pins `cph::io::Pxn`=> (ex: `cph::io::Pa0`)
    2. Inverted pins `cph::io::Pa0Inv`
    2. Control group of pins with `PinList` type `ph::io::PinList<cph::io::Pe11, cph::io::Pe12,cph:io::Pe13>`
    3. Convert pin type to port object `VPortBase` and pin object `VPin`
    4. Convert group of pins to  compile-time list: `typedef group_t::AsList compile_list_t`
*  _Example_:
<details> 
<summary> (Click to expand):</summary>
```c++
    //Control single pin
    typedef cph::io::Pe11 Led;
	Led::Port::Enable();
	Led::SetConfiguration(Led::Port::Out);
	Led::SetSpeed(Led::Speed::Slow);
	Led::Set();
```
```c++
	//Control group of pins
	typedef cph::io::PinList<cph::io::Pe11, cph::io::Pe12,cph:io::Pe13> group;
	group::SetConfiguration(group::Out);
	group::Write<0x11>();
```
```c++
//Create compile-time list with static check of uniqueness
typedef cph::io::PinList<cph::io::Pe11, cph::io::Pe12,cph::io::Pe13>::AsList pinlist_t
auto predicate2 = [] (auto element,auto tail) {
		typedef decltype (element) firstPin;
		auto comparer = [](auto element2){
			typedef decltype(element2) secondPin;
			return firstPin::Number==secondPin::Number;
		};
		return vd::find(tail,comparer);
	};
VOID_STATIC_ASSERT(vd::foreach(pinlist_t{},predicate2)==false);
```
</details>


## Plans
1. Make cmake files for stm32
2. Make examples for stm32
3. Rewrite wake protocol & add tests
4. Make Converter class for stream library
5. Remove unnecessary template abstraction -> split implementation, and move it to .cpp files
6. Add doxygen docs