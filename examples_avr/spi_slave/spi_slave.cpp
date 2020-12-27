#include <cph/spi.h>
#include <void/delay.h>
#include <void/interrupt.h>
using namespace cph;


uint8_t GlobalVar = 0;

//example of spi on interrupts
int main() {
	SpiSlaveAsync::init<SpiSpeed_4MHz>();
	SpiSlaveAsync::enable();
	SpiSlaveAsync::writeByte(0);
	//set data for trancieve, so on interrupt will be called trancieve callback;
	//on other way will be called receive_callback;
	SpiSlaveAsync::setCallback(nullptr, [](uint8_t* data ) {
		*data = GlobalVar++;
		GlobalVar %= 10;
		return true; //every time this callback returns true, SPI will be in sending mode, and will be called transmit callback
		//if return = false, then on next interrupt will be called receive callback
	});
	vd::EnableInterrupts();

	while (true) {
	}

	return 0;
}