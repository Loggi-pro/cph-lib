#pragma once
#ifdef _STM8
	#include "stm8/uart/usart_stm8.h"
#elif defined _AVR
	#include "platform/avr/spi/spi.h"
#elif defined WIN32
#endif
#include <void/static_assert.h>
#include <void/atomic.h>
#include "platform/fake/spi/spi_fake.h"
#include <cph/containers/ring_buffer.h>
#include "ioreg.h"

#ifndef F_CPU
	#error F_CPU not defined for cph/spi.h
#endif



namespace cph {
	namespace Spi {
		namespace Private {
			template <bool IsFake>
			struct SpiSelector {};


			template <>
			struct SpiSelector <true> {
				typedef FakeSpi_t selected;
				typedef SpiFakeHelper_t fakehelper;

			};


			#ifdef CPH_ENABLE_SPI
			template<>
			struct SpiSelector<false> {
				typedef cph::Spi::HardwareSpi0_t selected;
				typedef SpiFakeHelperStub_t fakehelper;
			};
			#endif


			template<uint8_t pin, class DEVICE_PORT, class DEVICE_DDR, bool IsFake>
			class TSpiBase {
				public:
					typedef typename Private::SpiSelector<IsFake>::selected Hardware;
					typedef typename Hardware::Flags Flags;
					typedef typename Private::SpiSelector<IsFake>::fakehelper Spy;
				protected:
					template <SpiSpeed SPEED, uint32_t FCPU = F_CPU, typename Flags::ByteOrder BYTE_ORDER = Flags::LsbFirst >
					static void _init() {
						const uint8_t prescalar = (FCPU / 1000000UL) / SPEED;
						const SpiPrescalar p = (SpiPrescalar)
						                       SpiPrescalarSelector<prescalar>::value; //change frequency of CPU\SPI
						Hardware::template init<p, pin, DEVICE_PORT, DEVICE_DDR, BYTE_ORDER>();
					}
					TSpiBase() {};
				public:

					static void deinit() {
						Hardware::deinit();
					}

					static void enable() {
						Hardware::enable();
					}
					static void disable() {
						Hardware::disable();
					}



			};


			template<uint8_t pin, class DEVICE_PORT, class DEVICE_DDR, bool IsFake>
			class SpiMasterBase: public TSpiBase<pin, DEVICE_PORT, DEVICE_DDR, IsFake> {
					SpiMasterBase() {}
					typedef TSpiBase<pin, DEVICE_PORT, DEVICE_DDR, IsFake> TBase;
				public:
					typedef typename TBase::Hardware Hardware;
					typedef typename TBase::Flags Flags;
					typedef typename TBase::Spy Spy;
					template <SpiSpeed SPEED, uint32_t FCPU = F_CPU, typename Flags::ByteOrder BYTE_ORDER = Flags::LsbFirst >
					static void init() {
						TBase::template _init<SPEED, FCPU, BYTE_ORDER>();
						Hardware::template setMaster<pin, DEVICE_PORT, DEVICE_DDR>();
					}

					template <class T>
					static bool readArray(uint8_t* arr, T size) {
						Hardware::template select<pin, DEVICE_PORT>();

						for (T index = 0; index < size; index++) {
							arr[index] = Hardware::readMasterBlocking();
						}

						Hardware::template release<pin, DEVICE_PORT>();
						return true;
					}
					static bool readByte(uint8_t* byte) {
						Hardware::template select<pin, DEVICE_PORT>();
						*byte = Hardware::readMasterBlocking();
						Hardware::template release<pin, DEVICE_PORT>();
						return true;
					}
					static bool writeByte(uint8_t byte) {
						Hardware::template select<pin, DEVICE_PORT>();
						Hardware::write(byte);

						while (!Hardware::writeReady()) { continue; }

						Hardware::template release<pin, DEVICE_PORT>();
						return true;
					}
					template <class T>
					static bool writeArray(uint8_t* arr, T size) {
						Hardware::template select<pin, DEVICE_PORT>();

						for (T index = 0; index < size; index++) {
							Hardware::write(arr[index]);

							while (!Hardware::writeReady()) { continue; }
						}

						Hardware::template release<pin, DEVICE_PORT>();
						return true;
					}
			};

			template<typename ProgramReg, SpiSpeed SPEED>
			class SoftwareSpiMasterBase: public
				TSpiBase<ProgramReg::Ss, typename ProgramReg::SPI_PORT, typename ProgramReg::SPI_DDR, false> {
					SoftwareSpiMasterBase() {}


				public:
					typedef typename ProgramReg::Flags Flags;
					typedef ProgramSpiRegActions<ProgramReg> Hardware;

					template <SpiSpeed _SPEED, uint32_t FCPU = F_CPU, typename Flags::ByteOrder BYTE_ORDER = Flags::LsbFirst >
					static void init() {
						Hardware::setMaster();
					}

					template <class T>
					static bool readArray(uint8_t* arr, T size) {
						Hardware::template select<SPEED>();

						for (T index = 0; index < size; index++) {
							arr[index] = Hardware::template readMasterBlocking< SPEED>();
						}

						Hardware::template release<SPEED>();
						return true;
					}
					static bool readByte(uint8_t* byte) {
						Hardware::template select<SPEED>();
						*byte = Hardware::template readMasterBlocking< SPEED>();
						Hardware::template release<SPEED>();
						return true;
					}
					static bool writeByte(uint8_t byte) {
						Hardware::template select<SPEED>();
						Hardware::template write< SPEED>(byte); //synchoronus
						//while (!Hardware::writeReady()) { continue; }
						Hardware::template release<SPEED>();
						return true;
					}
					template <class T>
					static bool writeArray(uint8_t* arr, T size) {
						Hardware::template select<SPEED>();

						for (T index = 0; index < size; index++) {
							Hardware::template write< SPEED>(arr[index]); //synchronous
							//while (!Hardware::writeReady()) { continue; }
						}

						Hardware::template release<SPEED>();
						return true;
					}


			};

			template<uint8_t pin, class DEVICE_PORT, class DEVICE_DDR, bool IsFake>
			class TSpiSlaveBase : public TSpiBase<pin, DEVICE_PORT, DEVICE_DDR, IsFake> {
					TSpiSlaveBase() {}
					typedef TSpiBase<pin, DEVICE_PORT, DEVICE_DDR, IsFake> TBase;
				public:
					typedef typename TBase::Hardware Hardware;
					typedef typename TBase::Flags Flags;
					typedef typename TBase::Spy Spy;
					template <SpiSpeed SPEED, uint32_t FCPU = F_CPU, typename Flags::ByteOrder BYTE_ORDER = Flags::LsbFirst >
					static void init() {
						TBase::template _init<SPEED, FCPU, BYTE_ORDER>();
						Hardware::setSlave();
					}

					template <class T>
					static bool readArray(uint8_t* arr, T size) {
						for (T index = 0; index < size; index++) {
							arr[index] = Hardware::readSlaveBlocking();
						}

						return true;
					}
					static bool readByte(uint8_t* byte) {
						*byte = Hardware::readSlaveBlocking();
						return true;
					}
					static bool writeByte(uint8_t byte) {
						Hardware::write(byte);

						while (!Hardware::writeReady()) { continue; }

						return true;
					}
					template <class T>
					static bool writeArray(uint8_t* arr, T size) {
						for (T index = 0; index < size; index++) {
							Hardware::write(arr[index]);

							while (!Hardware::writeReady()) { continue; }
						}

						return true;
					}
			};


			template<uint8_t pin, class DEVICE_PORT, class DEVICE_DDR, bool IsFake>
			class TAsyncSpiSlaveBase : public TSpiBase<pin, DEVICE_PORT, DEVICE_DDR, IsFake> {
					typedef TSpiBase<pin, DEVICE_PORT, DEVICE_DDR, IsFake> TBase;
					typedef void(*cbOnRecieve_t)(const uint8_t& value);
					typedef bool(*cbOnTrancieve_t)(uint8_t* data);
					static volatile bool _isSend ;
					static cbOnRecieve_t _onDataRecieveCallback;
					static cbOnTrancieve_t _onDataTrancieveCallback;
					TAsyncSpiSlaveBase() {};
				public:
					typedef typename Private::SpiSelector<IsFake>::selected Hardware;
					typedef typename Hardware::Flags Flags;
					typedef typename Private::SpiSelector<IsFake>::fakehelper Spy;
				private:
					static void _tranceiveHandler() {
						bool isDataExist = false;
						_isSend = false;
						uint8_t value;

						if (_onDataTrancieveCallback != nullptr) {
							isDataExist = _onDataTrancieveCallback(&value);
						};

						if (isDataExist) {
							Hardware::write(value);
							_isSend = true;
						};
					}
					static void _receiveHandler() {
						uint8_t value = Hardware::getByte();

						if (_onDataRecieveCallback != nullptr) {
							_onDataRecieveCallback(value);
						};
					}
				public:

					template <SpiSpeed SPEED, uint32_t FCPU = F_CPU, typename Flags::ByteOrder BYTE_ORDER = Flags::LsbFirst >
					static void init() {
						_onDataRecieveCallback = nullptr;
						_onDataTrancieveCallback = nullptr;
						TBase::template _init<SPEED, FCPU, BYTE_ORDER>();
						Hardware::setSlave();
					}

					static void deinit() {
						Hardware::disableInterrupt();
						Hardware::disable();
						Hardware::deinit();
					}


					static void enable() {
						Hardware::enableInterrupt();
						Hardware::enable();
					}
					static void disable() {
						Hardware::disableInterrupt();
						Hardware::disable();
					}
					//if thereis data for sending, will be called trancieve callback
					static bool writeByte(uint8_t byte) {
						vd::AtomicPolicy ::Set(&_isSend, true);
						Hardware::write(byte);
						return true;
					}
					static void IntHandler() {
						if (!Hardware::IsSpiInterrupt()) { return; }

						if (_isSend) {
							_tranceiveHandler();
						} else {
							_receiveHandler();
						}
					}



					static void setCallback(cbOnRecieve_t onDataRecieve,
					                        cbOnTrancieve_t  onDataTrancieve) {
						_onDataRecieveCallback = onDataRecieve;
						_onDataTrancieveCallback = onDataTrancieve;
					}
			};


			template<uint8_t pin, class DEVICE_PORT, class DEVICE_DDR, bool IsFake>
			typename TAsyncSpiSlaveBase<pin, DEVICE_PORT, DEVICE_DDR, IsFake>::cbOnRecieve_t
			TAsyncSpiSlaveBase<pin, DEVICE_PORT, DEVICE_DDR, IsFake>::_onDataRecieveCallback = nullptr;
			template<uint8_t pin, class DEVICE_PORT, class DEVICE_DDR, bool IsFake>
			typename TAsyncSpiSlaveBase<pin, DEVICE_PORT, DEVICE_DDR, IsFake>::cbOnTrancieve_t
			TAsyncSpiSlaveBase<pin, DEVICE_PORT, DEVICE_DDR, IsFake>::_onDataTrancieveCallback = nullptr;
			template<uint8_t pin, class DEVICE_PORT, class DEVICE_DDR, bool IsFake>
			volatile bool TAsyncSpiSlaveBase<pin, DEVICE_PORT, DEVICE_DDR, IsFake>::_isSend = false;


		}
	}



	//by default CS of Master device is SS of chip
	typedef Spi::Private::SpiMasterBase <
	/**/Spi::Private::SpiSelector< 0 >::selected::Regs::SsPin,
	/**/Spi::Private::SpiSelector< 0 >::selected::Regs::SPI_PORT,
	/**/Spi::Private::SpiSelector< 0 >::selected::Regs::SPI_DDR, false > TSpiMaster;
	typedef Spi::Private::TSpiSlaveBase <
	/**/Spi::Private::SpiSelector< 0 >::selected::Regs::SsPin,
	/**/Spi::Private::SpiSelector< 0 >::selected::Regs::SPI_PORT,
	/**/Spi::Private::SpiSelector< 0 >::selected::Regs::SPI_DDR, false > TSpiSlave;


	typedef Spi::Private::TAsyncSpiSlaveBase <
	/**/ Spi::Private::SpiSelector< 0 >::selected::Regs::SsPin,
	/**/ Spi::Private::SpiSelector< 0 >::selected::Regs::SPI_PORT,
	/**/ Spi::Private::SpiSelector< 0 >::selected::Regs::SPI_DDR, false > SpiSlaveAsync;


	typedef Spi::Private::TSpiBase < 0, NullReg<uint8_t>, NullReg<uint8_t>, true > TFakeSpi;

}