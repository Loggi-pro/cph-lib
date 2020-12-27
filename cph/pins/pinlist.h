#pragma once
#include <void/static_assert.h>
#include <void/meta.h> //compile-time list
#include <cph/loki/Typelist.h>
#include "iopin.h"
#include <void/static_if.h>
#include <void/auto_size.h>
#include <void/bits.h>
#include "pinlist_impl.h"
namespace cph {
	namespace io {
		namespace Private {
			template < unsigned int N, unsigned int MAX, class TEMP>
			struct _convert {
					template <typename Pins, typename L>
					struct _call {
							using list = L;
							class pinElement : public TypeAt<TEMP, N>::Result::Pin {};

							using result = typename _convert < N - 1, MAX, TEMP >::
							               template _call < Pins, decltype(vd::fn::push_front<pinElement>(L{})) >::result;
					};
			};
			//Specialized template
			template <unsigned int MAX, class TEMP>
			struct _convert<0, MAX, TEMP> {
					template <typename Pins, typename L>
					struct _call {
							class pinElement: public TypeAt<TEMP, 0>::Result::Pin {};
							using result = decltype(vd::fn::push_front<pinElement>(L{}));
					};
			};
		}

		//convert pinList to compiile-time list
		template <typename PinList, class TEMP>
		struct convertToList {
			enum {MAX = PinList::Length };
			using result = 	 typename Private::_convert < MAX - 1, MAX,
			      TEMP >::template _call<PinList, vd::fn::list<>>::result;
		};


		template<class PINS>
		class PinListProperties {
				typedef typename Private::GetPorts<PINS>::Result PinsToPorts;

				enum {LengthEnum = Loki::TL::Length<PINS>::value};
				enum {LastBitPositionEnum = Private::GetLastBitPosition<PINS>::value};
				typedef PINS PinTypeList;
			public:
				typedef typename Private::GetConfigPins<PINS>::Result ConfigPins;
				typedef typename Private::GetPorts<ConfigPins>::Result PinsToConfigPorts;

				typedef typename Loki::TL::NoDuplicates<PinsToPorts>::Result Ports;
				typedef typename Loki::TL::NoDuplicates<PinsToConfigPorts>::Result ConfigPorts;

				static const auto Length = LengthEnum;
				static const auto LastBitPosition = LastBitPositionEnum;

				enum {PortsHasSameConfig =
				          Private::CheckSameConfig<ConfigPorts>::value
				     };


				typedef vd::uintx_t < vd::bitsToMaxValue(LastBitPosition + 1) >  DataType;
		};

		////////////////////////////////////////////////////////////////////////////////
		// class template PinSet
		// Holds implimentation of pin list manipulations.
		// Pins from list are grouped by their port and group read/write operation is
		// performed on each port.
		////////////////////////////////////////////////////////////////////////////////

		template<class PINS>
		class PinSet : public PinListProperties<PINS>, public NativePortBase {
				typedef PinListProperties<PINS> Config;
			public:
				typedef typename Config::DataType ValueType;
				typedef typename Config::Ports Ports;
				typedef typename Config::ConfigPorts ConfigPorts;
				typedef typename Config::ConfigPins ConfigPins;
				typedef typename NativePortBase::Configuration PortConfiguration;

				typedef typename NativePortBase::Speed Speed;
				typedef typename NativePortBase::PullMode PullMode;
				typedef typename NativePortBase::DriverType DriverType;

				using Config::Length;




				template<uint8_t Num>
				class Take: public PinSet< typename Private::TakeFirst<PINS, Num>::Result >
				{};

				template<uint8_t Num>
				class Skip: public PinSet< typename Private::SkipFirst<PINS, Num>::Result >
				{};

				template<uint8_t StartIndex, uint8_t SliceSize>
				class Slice: public PinSet
					<
					typename Private::SkipFirst <
					typename Private::TakeFirst < PINS, uint8_t(StartIndex + SliceSize) >::Result,
					StartIndex >::Result
					> {
						VOID_STATIC_ASSERT(SliceSize == Slice::Length);
				};

				//convert to compilelist
				using AsList = typename convertToList<PinSet<PINS>, PINS>::result;



				template<uint8_t PIN>
				class Pin : public TypeAt<PINS, PIN>::Result::Pin
				{};

				static void Write(ValueType value);

				static ValueType Read();

				static void Set(ValueType value);

				static void Clear(ValueType value);

				static ValueType PinRead();

				// configuration with default mask
				static void SetConfiguration(PortConfiguration config);

				// configuration compatible with port definition
				static void SetConfiguration(ValueType mask, PortConfiguration config);

				static void SetSpeed(ValueType mask, Speed speed);
				static void SetPullUp(ValueType mask, PullMode pull);
				static void SetDriverType(ValueType mask, DriverType driver);
				static void AltFuncNumber(ValueType mask, uint8_t funcNumber);

				static void SetSpeed(Speed speed)             {SetSpeed(ValueType(-1), speed);}
				static void SetPullUp(PullMode pull)          {SetPullUp(ValueType(-1), pull);}
				static void SetDriverType(DriverType driver)  {SetDriverType(ValueType(-1), driver);}
				static void AltFuncNumber(uint8_t funcNumber) {AltFuncNumber(ValueType(-1), funcNumber);}

				// constant writing interface

				template<ValueType value>
				static void Write() {
					Private::PortWriteIterator<Ports, PINS, ValueType>:: template Write<value>();
				}

				template<ValueType value>
				static void Set() {
					Private::PortWriteIterator<Ports, PINS, ValueType>:: template Set<value>();
				}

				template<ValueType value>
				static void Clear() {
					Private::PortWriteIterator<Ports, PINS, ValueType>:: template Clear<value>();
				}

				template<ValueType mask, PortConfiguration config>
				static void SetConfiguration() {
					Private::PortConfigurationIterator<ConfigPorts, ConfigPins, PortConfiguration, config, ValueType>::
					template SetConfiguration<mask>();
				}

				template<PortConfiguration config>
				static void SetConfiguration() {
					Private::PortConfigurationIterator<ConfigPorts, ConfigPins, PortConfiguration, config, ValueType>::
					template SetConfiguration < ValueType(-1) > ();
				}


		};

		////////////////////////////////////////////////////////////////////////////////
		// PinList functions
		////////////////////////////////////////////////////////////////////////////////
		template<class PINS>
		void PinSet<PINS>::Write(typename PinSet<PINS>::ValueType value) {
			Private::PortWriteIterator<Ports, PINS, ValueType>::Write(value);
		}

		template<class PINS>
		typename PinSet<PINS>::ValueType PinSet<PINS>::Read() {
			typedef Private::PortWriteIterator<Ports, PINS, ValueType> iter;
			ValueType result = iter::OutRead();
			return result;
		}

		template<class PINS>
		void PinSet<PINS>::Set(typename PinSet<PINS>::ValueType value) {
			Private::PortWriteIterator<Ports, PINS, ValueType>::Set(value);
		}

		template<class PINS>
		void PinSet<PINS>::Clear(typename PinSet<PINS>::ValueType value) {
			Private::PortWriteIterator<Ports, PINS, ValueType>::Clear(value);
		}

		template<class PINS>
		typename PinSet<PINS>::ValueType PinSet<PINS>::PinRead() {
			typedef Private::PortWriteIterator<Ports, PINS, ValueType> iter;
			ValueType result = iter::PinRead();
			return result;
		}

		// configuration with default mask
		template<class PINS>
		void PinSet<PINS>::SetConfiguration(typename PinSet<PINS>::PortConfiguration config) {
			Private::PortWriteIterator<ConfigPorts, ConfigPins, ValueType>::SetConfiguration(config,
			        ValueType(-1));
		}

		// configuration compatible with port definition
		template<class PINS>
		void PinSet<PINS>::SetConfiguration(typename PinSet<PINS>::ValueType mask,
		                                    typename PinSet<PINS>::PortConfiguration config) {
			Private::PortWriteIterator<ConfigPorts, ConfigPins, ValueType>::SetConfiguration(config, mask);
		}

		template<class PINS>
		void PinSet<PINS>::SetSpeed(typename PinSet<PINS>::ValueType mask,
		                            typename PinSet<PINS>::Speed speed) {
			Private::PortWriteIterator<ConfigPorts, ConfigPins, ValueType>::SetSpeed(speed, mask);
		}

		template<class PINS>
		void PinSet<PINS>::SetPullUp(typename PinSet<PINS>::ValueType mask,
		                             typename PinSet<PINS>::PullMode pull) {
			Private::PortWriteIterator<ConfigPorts, ConfigPins, ValueType>::SetPullUp(pull, mask);
		}

		template<class PINS>
		void PinSet<PINS>::SetDriverType(typename PinSet<PINS>::ValueType mask,
		                                 typename PinSet<PINS>::DriverType driver) {
			Private::PortWriteIterator<ConfigPorts, ConfigPins, ValueType>::SetDriverType(driver, mask);
		}

		template<class PINS>
		void PinSet<PINS>::AltFuncNumber(typename PinSet<PINS>::ValueType mask, uint8_t funcNumber) {
			Private::PortWriteIterator<ConfigPorts, ConfigPins, ValueType>::AltFuncNumber(funcNumber, mask);
		}

		////////////////////////////////////////////////////////////////////////////////
		// class template MakePinList
		// This class is used to generate PinList and associate each pin in the list with
		// its bit position in value to be Write to and Read from pin list.
		////////////////////////////////////////////////////////////////////////////////

		template <
		    int Position,
		    typename T1  = NullType, typename T2  = NullType, typename T3  = NullType,
		    typename T4  = NullType, typename T5  = NullType, typename T6  = NullType,
		    typename T7  = NullType, typename T8  = NullType, typename T9  = NullType,
		    typename T10 = NullType, typename T11 = NullType, typename T12 = NullType,
		    typename T13 = NullType, typename T14 = NullType, typename T15 = NullType,
		    typename T16 = NullType, typename T17 = NullType, typename T18 = NullType,
		    typename T19 = NullType, typename T20 = NullType, typename T21 = NullType,
		    typename T22 = NullType, typename T23 = NullType, typename T24 = NullType,
		    typename T25 = NullType, typename T26 = NullType, typename T27 = NullType,
		    typename T28 = NullType, typename T29 = NullType, typename T30 = NullType,
		    typename T31 = NullType, typename T32 = NullType, typename T33 = NullType
		    >
		struct MakePinList {
			private:
				typedef typename MakePinList
				<
				Position + 1,
				         T2, T3, T4,
				         T5, T6, T7,
				         T8, T9, T10,
				         T11, T12, T13,
				         T14, T15, T16,
				         T17, T18, T19,
				         T20, T21, T22,
				         T23, T24, T25,
				         T26, T27, T28,
				         T29, T30, T31,
				         T32, T33
				         >
				         ::Result TailResult;
				enum {PositionInList = Position};
			public:
				typedef Typelist< Private::PinPositionHolder<T1, PositionInList>, TailResult> Result;

		};

		template<int Position>
		struct MakePinList<Position> {
			typedef NullType Result;
		};

		////////////////////////////////////////////////////////////////////////////////
		// class template PinList
		// Represents generic set of IO pins that could be used like a virtual port.
		// It can be composed from any number of pins from 1 to 16 from any IO port present in selected device
		// (the last T17 type in PinList is a end of list marker).
		// It can be used like this:
		//		typedef PinList<Pa0, Pa1, Pa2, Pa3, Pb5, Pb4, Pb2> pins;
		//		pins::Write(someValue);

		////////////////////////////////////////////////////////////////////////////////

		template
		<
		    typename T1  = NullType, typename T2  = NullType, typename T3  = NullType,
		    typename T4  = NullType, typename T5  = NullType, typename T6  = NullType,
		    typename T7  = NullType, typename T8  = NullType, typename T9  = NullType,
		    typename T10 = NullType, typename T11 = NullType, typename T12 = NullType,
		    typename T13 = NullType, typename T14 = NullType, typename T15 = NullType,
		    typename T16 = NullType, typename T17 = NullType, typename T18 = NullType,
		    typename T19 = NullType, typename T20 = NullType, typename T21 = NullType,
		    typename T22 = NullType, typename T23 = NullType, typename T24 = NullType,
		    typename T25 = NullType, typename T26 = NullType, typename T27 = NullType,
		    typename T28 = NullType, typename T29 = NullType, typename T30 = NullType,
		    typename T31 = NullType, typename T32 = NullType, typename T33 = NullType
		    >
		struct PinList: public PinSet
			<
			typename MakePinList
			<	0,	T1,
			T2, T3, T4,
			T5, T6, T7,
			T8, T9, T10,
			T11, T12, T13,
			T14, T15, T16,
			T17, T18, T19,
			T20, T21, T22,
			T23, T24, T25,
			T26, T27, T28,
			T29, T30, T31,
			T32, T33
			>::Result
			>
		{	};

	}
}


