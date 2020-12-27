#include "adc_impl.h"
#include <cph/platform/interrupt_tag_define.h>

extern "C"{
  	void ADC1_2_IRQHandler(void) {
		HAL_ADC_IRQHandler(&cph::Private::ADC1_Struct::handle);
	};


	void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* AdcHandle) {
		/* Get the converted value of regular channel */
			cph::Private::PlatformAdc::_IsConversionComplete = true;
		cph::Private::PlatformAdc::_ADCvalue = HAL_ADC_GetValue(AdcHandle);
	}

}