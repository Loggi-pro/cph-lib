#pragma once
#include <cph/timer/timer_types.h>
//FindedPrescalar = ::cph::Private::PreselectorIterator<PrescalarValue, true, 0>::value


namespace cph {
	namespace Private {
		template <uint32_t PERIOD, typename T, uint32_t FCPU>
		struct PwmFreqCalculator {
			private:
				static constexpr inline uint32_t max_ocr = (1UL << (sizeof(T) * 8)) - 1;
				static constexpr inline uint32_t maxPrescalar = FCPU / (2 *
				        PERIOD * (1 + max_ocr));
				/*static constexpr inline uint32_t maxPrescalar = FCPU / (PERIOD * (max_ocr + 1));*/


				PwmFreqCalculator() = delete;
			public:

				static constexpr inline cph::TimerPrescalar currentPrescalar
				    = (cph::TimerPrescalar)
				      (cph::Private::PreselectorIterator<maxPrescalar, true, 0 >::value);
				static constexpr  inline T ocrValue = FCPU / (2 * cph::TimerPrescalarValue<currentPrescalar>::value
				                                      *
				                                      PERIOD) - 1;
				/*static constexpr  inline T ocrValue = FCPU / (cph::TimerPrescalarValue<currentPrescalar>::value*
				                                      PERIOD);*/
		};
	}
}
