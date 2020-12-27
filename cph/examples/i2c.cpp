#include <cph/experimental/i2c/IIC_ultimate.h>
#include <cph/experimental/i2c/i2c_AT24C_EEP.h>
#include <void/interrupt.h>
#include <cph/basic_usart.h>



#define EEPROM_ADDR_PREFIX 0xA0
#define A2 0
#define A1 0
#define A0 0
#define EEPROM_ADDR ((EEPROM_ADDR_PREFIX) | (A2<<3)|(A1<<2)|(A0<<1))
volatile bool readCompleted = false;

void I2cComplete() {
	//complete
	readCompleted = true;
}

void I2cError() {
	//error
	readCompleted = true;
}



int main() {
	/*init_printf(printf_putchar);
	TUsart::init<19200>();
	I2c::init();
	vd::EnableInterrupts();
	printf("start\n");
	u08 data[10];

	for (u08 i = 0; i < sizeof(data); ++i) {
		data[i] = i;
		printf("Write byte[%d]:%d:", i, data[i]);
		readCompleted = false;

		if (!i2c_eep_WriteByte(EEPROM_ADDR, 0, data[i], 1, I2cComplete, I2cError)) {
			printf("busy\n");
		} else {
			while (!readCompleted) { continue; }
		}
	}

	readCompleted = false;
	bool res = i2c_eep_ReadSequence(EEPROM_ADDR, 0, data, 10, 1, I2cComplete);
	printf("ReadStart=%d\n", res);

	while (!readCompleted) { continue; }

	printf("Read:");

	for (u08 i = 0; i < sizeof(data); ++i) {
		printf(" %d", data[i]);
	}

	printf("\n");

	while (true) {
	}
	*/
	return 1;
}