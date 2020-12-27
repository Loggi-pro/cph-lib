#include <cph/basic_usart.h>
#include <void/interrupt.h>
typedef cph::UsartAsync<cph::Usart0> Usart;


extern "C" {
	VOID_ISR(CPH_INT_USART0_RXC) {
		Usart::Hardware::IntRxHandler();
	}

	VOID_ISR(CPH_INT_USART0_TXE) {
		Usart::Hardware::IntTxEmptyHandler();
	}
}


int main(void) {
	Usart::init<19200>();
	Usart::setOnReceive([](const uint8_t& value) {
		Usart::writeByte(value + 1);
	});
	vd::EnableInterrupts();

	while (true) {
		typename Usart::Flags::Error e = Usart::getError();

		if (e != Usart::Flags::NoError) {
			Usart::writeByte(e);
		}
	}

	return 0;
}