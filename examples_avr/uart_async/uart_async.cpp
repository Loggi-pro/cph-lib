#include <cph/basic_usart.h>
#include <void/interrupt.h>
typedef cph::UsartAsync<cph::Usart0> Usart;


int main() {
	Usart::init<19200>();
	Usart::setOnReceive([](const uint8_t& value) {
		Usart::writeByte((uint8_t)(value + 1));
	});
	vd::EnableInterrupts();

	while (true) {
		auto e = Usart::getError();

		if (e != Usart::Flags::NoError) {
			Usart::writeByte(e);
		}
	}
}