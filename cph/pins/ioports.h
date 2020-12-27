#pragma once
// Common base for all gpio ports

// Platform specific io ports implementation
#ifdef _ARM
	#include <cph/platform/stm32/core/platform_ports.h>
#endif
#ifdef _AVR
	#include <cph/platform/avr/core/platform_ports.h>
#endif
namespace cph {
	namespace io {
		class NullPort : public cph::io::NativePortBase {
			public:
				typedef uint8_t DataT;
				static void Write(DataT)
				{	}
				static void ClearAndSet(DataT, DataT)
				{	}
				static DataT Read() {
					return 0;
				}
				static void Set(DataT)
				{	}
				static void Clear(DataT)
				{	}
				static void Toggle(DataT)
				{	}
				static DataT PinRead() {
					return 0;
				}

				template<DataT clearMask, DataT>
				static void ClearAndSet()
				{}

				template<DataT>
				static void Toggle()
				{}

				template<DataT>
				static void Set()
				{}

				template<DataT>
				static void Clear()
				{}

				template<unsigned pin, class Config>
				static void SetPinConfiguration(Config)
				{}
				template<class Config>
				static void SetConfiguration(DataT, Config)
				{}

				template<DataT mask, Configuration>
				static void SetConfiguration()
				{}

				enum {Id = '-'};
				enum {Width = sizeof(DataT) * 8};
		};



	}
}
