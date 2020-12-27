#pragma once
#include <void/atomic.h>
#include <cph/ioreg.h>
#include <void/bits.h>
#include <void/static_assert.h>
namespace cph {
	namespace io {
		class NativePortBase {
			public:
				typedef uint8_t DataT;
				enum {Width = sizeof(DataT) * 8};
				static const unsigned MaxBitwiseOutput = 5;
			public:
				enum Configuration {
					Analog = 0,
					In = 0x00,
					Out = 0x01,
					AltFunc = 0x01
				};

				enum PullMode {
					NoPullUp = 0,
					PullUp   = 1,
					PullDown = 2
				};

				enum DriverType {
					PushPull  = 0,
					OpenDrain = 0
				};

				enum Speed {
					Slow    = 0,
					Fast    = 0,
					Fastest = 0
				};
		};

		//Port definitions for AtTiny, AtMega families.

		template<class Out, class Dir, class In, int ID>
		class PortImplimentation : public NativePortBase {
				template<DataT value, DataT mask>
				static inline void SetBitWise() {
					if (mask == 0) { return; }

					if (value & mask)
					{ Out::Or(value & mask); }

					SetBitWise < value, DataT(mask << 1) > ();
				}

				template<DataT value, DataT mask>
				static inline void ClearBitWise() {
					if (mask == 0) { return; }

					if (value & mask)
					{ Out::And(DataT(~(value & mask))); }

					ClearBitWise < value, DataT(mask << 1) > ();
				}
			public:
				static void Write(DataT value) {
					Out::Set(value);
				}

				static void ClearAndSet(DataT clearMask, DataT value) {
					//ATOMIC_BLOCK() {
					Out::AndOr(DataT(~clearMask), value);
					//	}
				}

				static DataT Read() {
					return Out::Get();
				}

				static void Set(DataT value) {
					//ATOMIC_BLOCK() {
					Out::Or(value);
					//}
				}

				static void Clear(DataT value) {
					//ATOMIC_BLOCK() {
					Out::And(~value);
					//}
				}

				static void Toggle(DataT value) {
					//ATOMIC_BLOCK() {
					Out::Xor(value);
					//}
				}

				static DataT PinRead() {
					return In::Get();
				}

				static void SetSpeed(DataT mask, Speed speed) {
					// nothing to do
				}

				static void SetPullUp(DataT mask, PullMode pull) {
					if (pull == PullUp) {
						//	ATOMIC_BLOCK() {
						Dir::And(DataT(~mask));
						Out::Or(mask);
						//	}
					}
				}

				static void SetDriverType(DataT mask, DriverType driver) {
					// nothing to do
				}

				static void AltFuncNumber(DataT mask, uint8_t number) {
					// nothing to do
				}

				// constant interface

				template<DataT clearMask, DataT value>
				static void ClearAndSet() {
					const DataT bitsToSet = value & clearMask;
					const DataT bitsToClear = DataT(~value & clearMask);
					const unsigned countBitsToChange = vd::PopulatedBits<clearMask>::value;

					if (countBitsToChange <= MaxBitwiseOutput &&
					        Id < 4) {
						SetBitWise<bitsToSet, 1>();
						ClearBitWise<bitsToClear, 1>();
					} else {
						//ATOMIC_BLOCK() {
						Out::AndOr(DataT(~clearMask), value);
						//}
					}
				}

				template<DataT value>
				static void Toggle() {
					//ATOMIC_BLOCK() {
					Out::Xor(value);
					//}
				}

				template<DataT value>
				static void Set() {
					if (vd::PopulatedBits<value>::value <= MaxBitwiseOutput &&
					        Id < 4)
					{ SetBitWise<value, 1>(); }
					else {
						//ATOMIC_BLOCK() {
						Out::Or(value);
						//}
					}
				}

				template<DataT value>
				static void Clear() {
					if (vd::PopulatedBits<value>::value <= MaxBitwiseOutput &&
					        Id < 4)
					{ ClearBitWise<value, 1>(); }
					else {
						//	ATOMIC_BLOCK() {
						Out::And(DataT(~value));
						//}
					}
				}

				// Configuration interface

				template<unsigned pin>
				static void SetPinConfiguration(Configuration configuration) {
					VOID_STATIC_ASSERT(pin < Width);

					if (configuration)
					{ Dir::Or(1 << pin); }
					else
					{ Dir::And(DataT(~(1 << pin))); }
				}

				static void SetConfiguration(DataT mask, Configuration configuration) {
					if (configuration)
					{ Dir::Or(mask); }
					else
					{ Dir::And(DataT(~mask)); }
				}

				template<DataT mask, Configuration configuration>
				static void SetConfiguration() {
					if (configuration) {
						if (mask == (DataT) - 1)
						{ Dir::Set(mask); }
						else
						{ Dir::Or(mask); }
					} else {
						if (mask == (DataT) - 1)
						{ Dir::Set(DataT(~mask)); }
						else
						{ Dir::And(DataT(~mask)); }
					}
				}

				static void Enable() {}
				static void Disable() {}

				enum {Id = ID};
		};
#include "__port_def.h"

	} // io

}// cph

