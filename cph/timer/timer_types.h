#pragma once
#include <void/chrono.h>
namespace cph {

	// ���� ����� ����� �������� ������������ ������� �� �������������� � ����� �� �����������, �� ����� �������� ��������� �������� =
	// ��� �������� ������������ ���� ����������� � ����� ���������=�������. ���� ������ ������������ ���, �� ���������� ��������=1

	enum TimerPrecision {
		Resolution_1ms,
		Resolution_100us,
		Resolution_10us,
	};



	enum TimerPrescalar {
		Stopped,
		Prescalar_1,
		Prescalar_8,
		Prescalar_64,
		Prescalar_256,
		Prescalar_1024,
		Prescalar_MaxIndex
	};

	template < template<TimerPrescalar> typename TYPE>
	constexpr uint16_t getTimerPrescalarBits(TimerPrescalar p) {
		switch (p) {
		case Stopped: return TYPE<Stopped>::Bits;

		case Prescalar_1: return TYPE<Prescalar_1>::Bits;

		case Prescalar_8: return TYPE<Prescalar_8>::Bits;

		case Prescalar_64: return TYPE<Prescalar_64>::Bits;

		case Prescalar_256: return TYPE<Prescalar_256>::Bits;

		case Prescalar_1024: return TYPE<Prescalar_1024>::Bits;

		default: return 0;
		};
	}

	inline constexpr uint16_t getPrescalarValues(TimerPrescalar p) {
		switch (p) {
		case Stopped: return 0;

		case Prescalar_1: return 1 ;

		case Prescalar_8: return 8;

		case Prescalar_64: return 64;

		case Prescalar_256: return 256;

		case Prescalar_1024: return 1024;

		case Prescalar_MaxIndex: return 1024;

		default: return 0;
		}
	}



	template <TimerPrescalar P>
	struct TimerPrescalarValue {};

	template <>
	struct TimerPrescalarValue<Stopped> {
		enum {
			value = 0,
		};
	};

	template <>
	struct TimerPrescalarValue<Prescalar_1> {
		enum {
			value = 1,
		};
	};
	template <>
	struct TimerPrescalarValue<Prescalar_8> {
		enum {
			value = 8,
		};
	};
	template <>
	struct TimerPrescalarValue<Prescalar_64> {
		enum {
			value = 64,
		};
	};
	template <>
	struct TimerPrescalarValue<Prescalar_256> {
		enum {
			value = 256UL,
		};
	};

	template <>
	struct TimerPrescalarValue<Prescalar_1024> {
		enum {
			value = 1024UL,
		};
	};



}



namespace cph {
	namespace Private {

		// ���� ����� ��� ��������������� ������� ������������
		// ���������� ���������� ������������ ��� ������� �������� �������� �������� �������� ������ � ��� ������ (���� ��� 2 �����)
		/**
		@struct	PrescalarSelector
		@brief	����� ������� ��������� ��������������� �� �������������
		PrescalarSelector<1>::value  == Prescalar_1
		PrescalarSelector<2>::value  == Prescalar_8
		PrescalarSelector<3>::value  == Prescalar_64
		PrescalarSelector<4>::value  == Prescalar_256
		PrescalarSelector<5>::value  == Prescalar_1024
		*/
		template<uint8_t index>
		struct PrescalarSelector {};

		template <>
		struct PrescalarSelector<cph::Prescalar_1024> {
			enum {
				value = cph::Prescalar_1024
			};
		};
		template <>
		struct PrescalarSelector<cph::Prescalar_256> {
			enum {
				value = cph::Prescalar_256
			};
		};
		template <>
		struct PrescalarSelector<cph::Prescalar_64> {
			enum {
				value = cph::Prescalar_64
			};
		};
		template <>
		struct PrescalarSelector<cph::Prescalar_8> {
			enum {
				value = cph::Prescalar_8
			};
		};
		template <>
		struct PrescalarSelector<cph::Prescalar_1> {
			enum {
				value = cph::Prescalar_1
			};
		};


		/**
		@struct	PrescalarComparator
		@brief	CompileTime ��������� �������� ������������ BASE_VALUE � ����� �� ������������� �� ������� N
		PrescalarComparator<1024,1>::result
		*/

		template <uint32_t BASE_VALUE, uint8_t N>
		struct PrescalarComparator {
			enum {
				result = BASE_VALUE > TimerPrescalarValue<cph::TimerPrescalar(PrescalarSelector<N>::value)>::value
			};
		};



		/**
		@struct	PreselectorIterator
		@brief	Declare iterator throught an array of PrescalarSelector
		@param BASE_VALUE - ������������, ����� ����� ������ ���� � �������
		@param IsTryNext - ���� true �� ����������� ��������� ������������ � �������, ���� false �� ������ ������� ������������ ������ ��� BASE_VALUE

		*/



		template <uint32_t BASE_VALUE, bool IsTryNext, uint8_t N>
		struct PreselectorIterator {};
		template <uint32_t BASE_VALUE, bool IsTryNext, uint8_t N>
		struct PreselectorIteratorReverse {};

		/**
		@struct	PreselectorIterator
		@brief	First Step:		Check that this next element is lower that BASE.
		Based on this condition selected one of three variants
		Variant 1. Repeat this step.

		*/

		template <uint32_t BASE_VALUE, uint8_t N>
		struct PreselectorIterator<BASE_VALUE, true, N> {
			enum {
				value = PreselectorIterator < BASE_VALUE, PrescalarComparator < BASE_VALUE, (uint8_t)(N + 1) >::result,
				(uint8_t)(N + 1) >::value
			};

		};


		template <uint32_t BASE_VALUE, uint8_t N>
		struct PreselectorIteratorReverse<BASE_VALUE, true, N> {
			enum {
				value = PreselectorIterator < BASE_VALUE, PrescalarComparator < BASE_VALUE, (uint8_t)(N - 1) >::result,
				(uint8_t)(N - 1) >::value
			};

		};

		/**
		@struct	PreselectorIterator
		@brief			Variant 2: Current Prescalar is bigger than BASE. Then return value of this prescalar

		*/
		template <uint32_t BASE_VALUE, uint8_t N>
		struct PreselectorIterator<BASE_VALUE, false, N> {
			enum {
				value = PrescalarSelector<N>::value
			};

		};

		template <uint32_t BASE_VALUE, uint8_t N>
		struct PreselectorIteratorReverse<BASE_VALUE, false, N> {
			enum {
				value = PrescalarSelector<N>::value
			};

		};

		/**
		@struct	PreselectorIterator
		@brief			Variant 3: Prescalar is still bigger,then return Stopped and catch it for OVERFLOW

		*/
		template <uint32_t BASE_VALUE>
		struct PreselectorIterator < BASE_VALUE, true, cph::Prescalar_1024> {
			enum {
				value = cph::TimerPrescalar::Stopped
			};
			//Too big interval, try to make interval lesser, or decrease MCU frequency (F_CPU)
		};

		template <uint32_t BASE_VALUE>
		struct PreselectorIteratorReverse < BASE_VALUE, true, cph::Prescalar_1> {
			enum {
				value = cph::TimerPrescalar::Prescalar_1
			};
			//Too big interval, try to make interval lesser, or decrease MCU frequency (F_CPU)
		};


	}
}
