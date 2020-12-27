#include <cph/basic_usart.h>
#include <void/interrupt.h>
#include <cph/system_counter.h>
#include <void/delay.h>
#include <cph/adc.h>
#include <cph/timer.h>
#include <cph/iostream.h>
#include <cph/pwm.h>
#include <cph/gpio.h>
typedef cph::basic_ostream<cph::AdapterUsartSync_Debug<cph::Usart0>> ostream;
ostream cout;



VOID_ISR(CPH_INT_TIMER0_OCRA) {
	cph::Private::timer0::THwTimer0::IntOCRA_Handler();
}



template<typename pin>
void pinSetOutput() {
	pin::Port::Enable();
	pin::SetConfiguration(pin::Port::Out);
}
template<typename pin>
void pinSetInput() {
	pin::Port::Enable();
	pin::SetConfiguration(pin::Port::In);
}
using pinAdc = cph::io::Pc0;
using pinPwm = cph::io::Pb1;
using pinLed = cph::io::Pb5;
int main(void) {
	cph::TSystemCounter::initPrecision<cph::TimerPrecision::Resolution_1ms, cph::TIMER_0>();
	cph::TSystemCounter::enable<cph::TIMER_0>();
	vd::EnableInterrupts();
	pinSetOutput<pinLed>();
	pinSetOutput<pinPwm>();
	pinSetInput<pinAdc>();
	cout << "init\n";
	cph::TPwm::initPwm<cph::TIMER_1, cph::PwmChannel::Channel_0, cph::TimerPrescalar::Prescalar_1>();
	cph::TPwm::setValuePercent<cph::TIMER_1, cph::PwmChannel::Channel_0>(50);
	cph::Adc::init<pinAdc>(cph::Adc::AdcDivider::Div64);
	cph::timer_t timer;
	cout << "start\n";

	while (true) {
		if (timer.isElapsed(vd::chrono::milli_t(100))) {
			pinLed::Toggle();
			timer.start();
			uint16_t a = cph::Adc::singleConversion();
			uint32_t skvagnost = (a * 10) / 102;
			cout << a << cph::endl;
			cph::TPwm::setValuePercent<cph::TIMER_1, cph::PwmChannel::Channel_0>(skvagnost);
		}
	}

	return 0;
}