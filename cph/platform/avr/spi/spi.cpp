#include <cph/spi.h>
#include <cph/platform/interrupt_tag_define.h>
#ifdef CPH_EABLE_SPI
extern "C" {
	VOID_ISR(CPH_INT_SPI) {
		PROVIDE_ENTRY(CPH_SPI_ISR);
		SpiSlaveAsync::IntHandler();
	}
}
#endif