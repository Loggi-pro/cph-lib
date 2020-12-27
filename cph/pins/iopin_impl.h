namespace cph {
	namespace io {
		template<class PORT, uint8_t PIN, class CONFIG_PORT>
		void TPin<PORT, PIN, CONFIG_PORT>::Set() {
			PORT::template Set < 1u << PIN > ();
		}

		template<class PORT, uint8_t PIN, class CONFIG_PORT>
		void TPin<PORT, PIN, CONFIG_PORT>::Set(bool val) {
			if (val) {
				Set();
			} else {
				Clear();
			}
		}


		template<class PORT, uint8_t PIN, class CONFIG_PORT>
		void TPin<PORT, PIN, CONFIG_PORT>::SetDir(uint8_t val) {
			if (val)
			{ SetDirWrite(); }
			else { SetDirRead(); }
		}

		template<class PORT, uint8_t PIN, class CONFIG_PORT>
		void TPin<PORT, PIN, CONFIG_PORT>::Clear() {
			PORT::template Clear < 1u << PIN > ();
		}

		template<class PORT, uint8_t PIN, class CONFIG_PORT>
		void TPin<PORT, PIN, CONFIG_PORT>::Toggle() {
			PORT::Toggle(1u << PIN);
		}

		template<class PORT, uint8_t PIN, class CONFIG_PORT>
		void TPin<PORT, PIN, CONFIG_PORT>::SetDirRead() {
			PORT::template SetPinConfiguration<PIN>( PORT::In);
		}

		template<class PORT, uint8_t PIN, class CONFIG_PORT>
		void TPin<PORT, PIN, CONFIG_PORT>::SetDirWrite() {
			ConfigPort:: template SetPinConfiguration<PIN>(PORT::Out);
		}

		template<class PORT, uint8_t PIN, class CONFIG_PORT>
		void TPin<PORT, PIN, CONFIG_PORT>::SetConfiguration(Configuration configuration) {
			ConfigPort:: template SetPinConfiguration<PIN>(configuration);
		}

		template<class PORT, uint8_t PIN, class CONFIG_PORT>
		void TPin<PORT, PIN, CONFIG_PORT>::SetDriverType(DriverType driverType) {
			ConfigPort::SetDriverType(1u << PIN, driverType);
		}

		template<class PORT, uint8_t PIN, class CONFIG_PORT>
		void TPin<PORT, PIN, CONFIG_PORT>::SetPullUp(PullMode pullMode) {
			ConfigPort::SetPullUp(1u << PIN, pullMode);
		}

		template<class PORT, uint8_t PIN, class CONFIG_PORT>
		void TPin<PORT, PIN, CONFIG_PORT>::SetSpeed(Speed speed) {
			ConfigPort::SetSpeed(1u << PIN, speed);
		}

		template<class PORT, uint8_t PIN, class CONFIG_PORT>
		void TPin<PORT, PIN, CONFIG_PORT>::AltFuncNumber(uint8_t funcNumber) {
			ConfigPort::AltFuncNumber(1u << PIN, funcNumber);
		}

		template<class PORT, uint8_t PIN, class CONFIG_PORT>
		bool TPin<PORT, PIN, CONFIG_PORT>::IsSet() {
			return (PORT::PinRead() & (typename PORT::DataT)(1u << PIN)) != 0;
		}
		template<class PORT, uint8_t PIN, class CONFIG_PORT>
		bool TPin<PORT, PIN, CONFIG_PORT>::IsOutSet() {
			return (PORT::Read() & (typename PORT::DataT)(1u << PIN)) != 0;
		}

		template<class PORT, uint8_t PIN, class CONFIG_PORT>
		void TPin<PORT, PIN, CONFIG_PORT>::WaiteForSet() {
			while (IsSet() == 0) {}
		}

		template<class PORT, uint8_t PIN, class CONFIG_PORT>
		void TPin<PORT, PIN, CONFIG_PORT>::WaiteForClear() {
			while (IsSet()) {}
		}

		template<class PORT, uint8_t PIN, class CONFIG_PORT>
		void InvertedPin<PORT, PIN, CONFIG_PORT>::Set(bool val) {
			if (val)
			{ PORT::template Clear < 1u << PIN > (); }
			else
			{ PORT::template Set < 1u << PIN > (); }
		}

		template<class PORT, uint8_t PIN, class CONFIG_PORT>
		void InvertedPin<PORT, PIN, CONFIG_PORT>::Set() {
			Set(true);
		}

		template<class PORT, uint8_t PIN, class CONFIG_PORT>
		void InvertedPin<PORT, PIN, CONFIG_PORT>::Clear() {
			Set(false);
		}

		template<class PORT, uint8_t PIN, class CONFIG_PORT>
		bool InvertedPin<PORT, PIN, CONFIG_PORT>::IsSet() {
			return (PORT::PinRead() & (typename PORT::DataT)(1u << PIN)) == 0;
		}

	}
}
