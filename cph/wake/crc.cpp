#include "crc.h"


void Calc_crc8(u08* varCrc, u08 POLYNOM, u08 data) {
	u08 counter = 8;
	u08 flag = 0;
	*varCrc ^= data;

	while (counter > 0) {
		flag = (*varCrc) & (1 << 7);
		*varCrc <<= 1;

		if (flag) { *varCrc ^= POLYNOM; }

		counter--;
	}
}

u08 Calc_crc8_arr(u08 Poly,u08*arr,u08 len,u08 init_data){
	u08 result = init_data;
	for (u08 i = 0;i<len;i++){
		Calc_crc8(&result,Poly,*arr);
		arr++;
	}
	return result;
}
