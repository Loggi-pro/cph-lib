#include "platform_i2c.h"

extern "C" {
#ifdef CPH_HAS_I2C_1
void I2C1_EV_IRQHandler(void)
{
  cph::Private::i2c_hardware_t<1>::IntHandlerEvent();
}

void I2C1_ER_IRQHandler(void)
{
  cph::Private::i2c_hardware_t<1>::IntHandlerError();
}
		
#endif
		
	
	
}