#pragma once

#include <void/static_assert.h>

namespace cph {
	namespace io {
// this class represents one pin in a IO port.
// It is fully static.
		template<class PORT, uint8_t PIN, class CONFIG_PORT = PORT>
		class TPin {
				VOID_STATIC_ASSERT(PIN < PORT::Width);
			public:
				typedef PORT Port;
				typedef CONFIG_PORT ConfigPort;

				typedef typename ConfigPort::Speed Speed;
				typedef typename ConfigPort::PullMode PullMode;
				typedef typename ConfigPort::DriverType DriverType;
				typedef typename ConfigPort::Configuration Configuration;

				static const unsigned Number = PIN;
				static const bool Inverted = false;

				static void Set();
				static void Set(bool val);
				static void SetDir(uint8_t val);

				static void Clear();
				static void Toggle();
				static void SetDirRead();
				static void SetDirWrite();
				static void SetConfiguration(Configuration configuration);

				static void SetDriverType(DriverType driverType);
				static void SetPullUp(PullMode pullMode);
				static void SetSpeed(Speed speed);
				static void AltFuncNumber(uint8_t funcNumber);

				template<Configuration configuration>
				static void SetConfiguration() {
					ConfigPort:: template SetConfiguration < 1 << PIN, configuration > ();
				}

				static bool IsSet();
				static bool IsOutSet();
				static void WaiteForSet();
				static void WaiteForClear();
		};

		template<class PORT, uint8_t PIN, class CONFIG_PORT = PORT>
class InvertedPin : public TPin<PORT, PIN, CONFIG_PORT> {
public:
			static const bool Inverted = true;

			static void Set(bool val);
			static void Set();
			static void Clear();
			static bool IsSet();
		};

	}
}
#include "iopin_impl.h"
