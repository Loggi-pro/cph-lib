#include <cph/spi.h>
#include <void/delay.h>
#include <void/interrupt.h>
#include <cph/ioreg.h>
using namespace cph;


IO_REG_WRAPPER(DDRB, ddrb, uint8_t);
IO_REG_WRAPPER(PORTB, portb, uint8_t);

//*** Program Master NOT TESTED ***///
//LSByteOrder NYI
//All pins must be on same port
IO_REG_WRAPPER(DDRC, SPI_DDR, uint8_t);
IO_REG_WRAPPER(PORTC, SPI_PORT, uint8_t);
using SpiReg =  Spi::Private::ProgramReg<SPI_DDR, SPI_PORT, PC0, PC1, PC2, PC3> ;
//==============================================================================
using SelectedSpi = Spi::Private::TSpiMasterBase< PB0, portb, ddrb, false>; //Spi with CS on pin PB0
//using SelectedSpi = Spi::Private::TProgramSpiMasterBase< SpiReg, SpiSpeed_4MHz>; //Spi with CS on default (SS) pin;

int main(void) {
	SelectedSpi::init<SpiSpeed_4MHz, F_CPU, SelectedSpi::Flags::LsbFirst>();
	SelectedSpi::enable();
	uint8_t i = 0;
	DDRC = 0xFF;

	while (true) {
		if (SelectedSpi::writeByte(i)) {
			PORTC = i;
		}

		vd::delay_ms<500>();
	}

	return 0;
}