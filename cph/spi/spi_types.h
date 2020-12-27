#pragma once
namespace cph {
enum SpiSpeed {
	SpiSpeed_16MHz = 16,
	SpiSpeed_8MHz = 8,
	SpiSpeed_4MHz = 4,
	SpiSpeed_2MHz = 2,
};
namespace Spi {

namespace Private {

enum SpiPrescalar {
	SpiPrescalar_2,
	SpiPrescalar_4,
	SpiPrescalar_8,
	SpiPrescalar_16,
	SpiPrescalar_32,
	SpiPrescalar_64,
	SpiPrescalar_128,
	SpiPrescalar_MaxIndex
};




template <SpiPrescalar P>
struct SpiPrescalarValue {};

template <>
struct SpiPrescalarValue<SpiPrescalar_2> {
	enum {
		value = 2,
	};
};
template <>
struct SpiPrescalarValue<SpiPrescalar_4> {
	enum {
		value = 4,
	};
};
template <>
struct SpiPrescalarValue<SpiPrescalar_8> {
	enum {
		value = 8,
	};
};
template <>
struct SpiPrescalarValue<SpiPrescalar_16> {
	enum {
		value = 16,
	};
};
template <>
struct SpiPrescalarValue<SpiPrescalar_32> {
	enum {
		value = 32,
	};
};
template <>
struct SpiPrescalarValue<SpiPrescalar_64> {
	enum {
		value = 64,
	};
};
template <>
struct SpiPrescalarValue<SpiPrescalar_128> {
	enum {
		value = 128,
	};
};


// Этот класс для автоматического расчета предделителя
/**
@struct	SpiPrescalarSelector
@brief	Класс который позволяет индексироваться по предделителям
SpiPrescalarSelector<2>::value  == SpiPrescalar_2
SpiPrescalarSelector<4>::value  == SpiPrescalar_4
SpiPrescalarSelector<8>::value  == SpiPrescalar_8
SpiPrescalarSelector<16>::value  == SpiPrescalar_16
SpiPrescalarSelector<32>::value  == SpiPrescalar_32
SpiPrescalarSelector<64>::value  == SpiPrescalar_64
SpiPrescalarSelector<128>::value  == SpiPrescalar_128
*/


template<uint8_t index>
struct SpiPrescalarSelector {};

template<>
struct SpiPrescalarSelector<2> {
	enum {
		value = SpiPrescalar_2,
	};
};
template<>
struct SpiPrescalarSelector<4> {
	enum {
		value = SpiPrescalar_4,
	};
};

template<>
struct SpiPrescalarSelector<8> {
	enum {
		value = SpiPrescalar_8,
	};
};
template<>
struct SpiPrescalarSelector<32> {
	enum {
		value = SpiPrescalar_32,
	};
};
template<>
struct SpiPrescalarSelector<64> {
	enum {
		value = SpiPrescalar_64,
	};
};
template<>
struct SpiPrescalarSelector<128> {
	enum {
		value = SpiPrescalar_128,
	};
};
}
}
}