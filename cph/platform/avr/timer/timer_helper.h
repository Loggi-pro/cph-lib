#pragma once

#include <void/chrono.h>
#include <void/meta.h>
#include <cph/timer/timer_types.h>

namespace cph::Private::timer {
	struct prescalar_info_t {
		TimerPrescalar prescalar;
		uint8_t ocrValue;
	};

	template<uint32_t MAX_OCR, uint32_t FCPU, typename Period>
	constexpr prescalar_info_t calculatePrescalar(const vd::chrono::any_t<Period> &tickTime) {
		typedef vd::chrono::any_t<Period> TimeTypedef;
		uint32_t MaxPrescalarValue = tickTime.count() * TimeTypedef::PeriodInfo::Num *
									 (FCPU / (1000UL * TimeTypedef::PeriodInfo::Denom)) / MAX_OCR;
		constexpr auto prescalarList = vd::fn::generate<TimerPrescalar::Stopped, 1,
				TimerPrescalar::Prescalar_MaxIndex - 1>();
		size_t index = vd::fn::find_if(prescalarList, [MaxPrescalarValue](auto t) -> bool {
			using type = typename decltype(t)::type;
			auto v = getPrescalarValues((TimerPrescalar) type::value);
			return v > MaxPrescalarValue;
		});
		assert(index > 0 && index < TimerPrescalar::Prescalar_MaxIndex);
		auto prescalar = TimerPrescalar(index);
		auto prescalarValue = getPrescalarValues(prescalar);
		const uint8_t ocrValue = (uint8_t) (
				(uint32_t) ((FCPU) / (1000UL * Period::Denom) * Period::Num * tickTime.count()) /
				(prescalarValue));
		prescalar_info_t Result = {prescalar,   //prescalar
								   ocrValue // ocrValue
		};
		return Result;
	}

}

