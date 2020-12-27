#pragma once
#include <cph/ioreg.h>
#include <cph/spi/spi_types.h>
#include <void/delay.h>
#include <assert.h>
namespace cph {
	namespace Spi {
		namespace Private {
			#if defined(__AVR_ATmega32__)
#define CPH_HAS_SPI0 true
#define CPH_INT_SPI SPI_STC_vect


			template <SpiPrescalar P>
			struct SpiPrescalarData {};

			template <>
			struct SpiPrescalarData<SpiPrescalar_2> {
				enum {
					Spi2x = (1 << SPI2X),
					SPR = (0 << SPR1) | (0 << SPR0),
					Spi2xMask = (1 << SPI2X),
					SPRMask = (1 << SPR1) | (1 << SPR0),
				};
			};
			template <>
			struct SpiPrescalarData<SpiPrescalar_4> {
				enum {
					Spi2x = (0 << SPI2X),
					SPR = (0 << SPR1) | (0 << SPR0),
					Spi2xMask = (1 << SPI2X),
					SPRMask = (1 << SPR1) | (1 << SPR0),
				};
			};
			template <>
			struct SpiPrescalarData<SpiPrescalar_8> {
				enum {
					Spi2x = (1 << SPI2X),
					SPR = (0 << SPR1) | (1 << SPR0),
					Spi2xMask = (1 << SPI2X),
					SPRMask = (1 << SPR1) | (1 << SPR0),
				};
			};
			template <>
			struct SpiPrescalarData<SpiPrescalar_16> {
				enum {
					Spi2x = (0 << SPI2X),
					SPR = (0 << SPR1) | (1 << SPR0),
					Spi2xMask = (1 << SPI2X),
					SPRMask = (1 << SPR1) | (1 << SPR0),
				};
			};
			template <>
			struct SpiPrescalarData<SpiPrescalar_32> {
				enum {
					Spi2x = (1 << SPI2X),
					SPR = (1 << SPR1) | (0 << SPR0),
					Spi2xMask = (1 << SPI2X),
					SPRMask = (1 << SPR1) | (1 << SPR0),
				};
			};
			template <>
			struct SpiPrescalarData<SpiPrescalar_64> {
				enum {
					Spi2x = (1 << SPI2X),
					SPR = (1 << SPR1) | (1 << SPR0),
					Spi2xMask = (1 << SPI2X),
					SPRMask = (1 << SPR1) | (1 << SPR0),
				};
			};
			template <>

			struct SpiPrescalarData<SpiPrescalar_128> {
				enum {
					Spi2x = (0 << SPI2X),
					SPR = (1 << SPR1) | (1 << SPR0),
					Spi2xMask = (1 << SPI2X),
					SPRMask = (1 << SPR1) | (1 << SPR0),
				};
			};


			class SpiReg {
				public:
					IO_REG_WRAPPER(DDRB, SPI_DDR, uint8_t);
					IO_REG_WRAPPER(PORTB, SPI_PORT, uint8_t);
					IO_REG_WRAPPER(SPCR, REG_SPCR, uint8_t);
					IO_REG_WRAPPER(SPSR, REG_SPSR, uint8_t);
					IO_REG_WRAPPER(SPDR, REG_SPDR, uint8_t);
					enum {
						Mosi = 1 << PB5,
						Miso = 1 << PB6,
						Sck = 1 << PB7,
						SsPin = PB4,
						//
						SpiePin = SPIE,
						MstrPin = MSTR,
						SpePin = SPE,
						SpifPin = SPIF,

					};
					struct Flags {
						enum ByteOrder {
							LsbFirst = 1 << DORD,
							MsbFirst = 0 << DORD,
						};
					};


			};
			#elif defined(__AVR_ATmega168P__)
#define CPH_HAS_SPI0 true
#define CPH_INT_SPI SPI_STC_vect


			template <SpiPrescalar P>
			struct SpiPrescalarData {};

			template <>
			struct SpiPrescalarData<SpiPrescalar_2> {
				enum {
					Spi2x = (1 << SPI2X),
					SPR = (0 << SPR1) | (0 << SPR0),
					Spi2xMask = (1 << SPI2X),
					SPRMask = (1 << SPR1) | (1 << SPR0),
				};
			};
			template <>
			struct SpiPrescalarData<SpiPrescalar_4> {
				enum {
					Spi2x = (0 << SPI2X),
					SPR = (0 << SPR1) | (0 << SPR0),
					Spi2xMask = (1 << SPI2X),
					SPRMask = (1 << SPR1) | (1 << SPR0),
				};
			};
			template <>
			struct SpiPrescalarData<SpiPrescalar_8> {
				enum {
					Spi2x = (1 << SPI2X),
					SPR = (0 << SPR1) | (1 << SPR0),
					Spi2xMask = (1 << SPI2X),
					SPRMask = (1 << SPR1) | (1 << SPR0),
				};
			};
			template <>
			struct SpiPrescalarData<SpiPrescalar_16> {
				enum {
					Spi2x = (0 << SPI2X),
					SPR = (0 << SPR1) | (1 << SPR0),
					Spi2xMask = (1 << SPI2X),
					SPRMask = (1 << SPR1) | (1 << SPR0),
				};
			};
			template <>
			struct SpiPrescalarData<SpiPrescalar_32> {
				enum {
					Spi2x = (1 << SPI2X),
					SPR = (1 << SPR1) | (0 << SPR0),
					Spi2xMask = (1 << SPI2X),
					SPRMask = (1 << SPR1) | (1 << SPR0),
				};
			};
			template <>
			struct SpiPrescalarData<SpiPrescalar_64> {
				enum {
					Spi2x = (1 << SPI2X),
					SPR = (1 << SPR1) | (1 << SPR0),
					Spi2xMask = (1 << SPI2X),
					SPRMask = (1 << SPR1) | (1 << SPR0),
				};
			};
			template <>

			struct SpiPrescalarData<SpiPrescalar_128> {
				enum {
					Spi2x = (0 << SPI2X),
					SPR = (1 << SPR1) | (1 << SPR0),
					Spi2xMask = (1 << SPI2X),
					SPRMask = (1 << SPR1) | (1 << SPR0),
				};
			};


			class SpiReg {
				public:
					IO_REG_WRAPPER(DDRB, SPI_DDR, uint8_t);
					IO_REG_WRAPPER(PORTB, SPI_PORT, uint8_t);
					IO_REG_WRAPPER(SPCR, REG_SPCR, uint8_t);
					IO_REG_WRAPPER(SPSR, REG_SPSR, uint8_t);
					IO_REG_WRAPPER(SPDR, REG_SPDR, uint8_t);
					enum {
						Mosi = 1 << PB3,
						Miso = 1 << PB4,
						Sck = 1 << PB5,
						SsPin = PB2,
						//
						SpiePin = SPIE,
						MstrPin = MSTR,
						SpePin = SPE,
						SpifPin = SPIF,

					};
					struct Flags {
						enum ByteOrder {
							LsbFirst = 1 << DORD,
							MsbFirst = 0 << DORD,
						};
					};


			};

			#endif



			template <typename _SPI_DDR, typename _SPI_PORT, uint8_t MosiBit, uint8_t MisoBit, uint8_t SckBit, uint8_t SsBit = 0>
			class Registers {
				public:
					typedef _SPI_DDR SPI_DDR;
					typedef _SPI_PORT SPI_PORT;
					//IO_REG_WRAPPER((uint8_t)_SPI_DDR, SPI_DDR, uint8_t);
					//IO_REG_WRAPPER((uint8_t)_SPI_PORT, SPI_PORT, uint8_t);
					enum {
						Mosi = 1 << MosiBit,
						Miso = 1 << MisoBit,
						Sck = 1 << SckBit,
						Ss = 1 << SsBit
					};
					struct Flags {
						enum ByteOrder {
							LsbFirst = 1,
							MsbFirst = 0,
						};
					};


			};

			template<class REG>
			class SpiRegActions {
				public:
					typedef REG Regs;
					typedef typename REG::Flags Flags;
					template <uint8_t pin, class DEVICE_PORT, class DEVICE_DDR>
					static void setMaster() {
						REG::SPI_DDR::Or(REG::Mosi | REG::Sck);
						REG::SPI_DDR::And((uint8_t)~REG::Miso);
						DEVICE_PORT::template BitSet<pin>();
						DEVICE_DDR::template BitSet<pin>(); // SS as output
						REG::REG_SPCR::template BitSet<REG::MstrPin>();
					}
					static void setSlave() {
						REG::SPI_DDR::AndNot(REG::Mosi | REG::Sck); //as input
						REG::SPI_PORT::AndNot(REG::Miso);
						REG::SPI_DDR::Or(REG::Miso); //as output
						REG::SPI_DDR::template BitClear<REG::SsPin>(); //SS=1,SS as input
						REG::SPI_PORT::template BitSet<REG::SsPin>(); //SS=1,SS as input
						REG::REG_SPCR::template BitClear<REG::MstrPin>();
					}
					static void enable() {
						REG::REG_SPCR::template BitSet<REG::SpePin>();
					}

					static void disable() {
						REG::REG_SPCR::template BitClear<REG::SpePin>();
					}

					template <SpiPrescalar Prescalar, uint8_t pin, class DEVICE_PORT, class DEVICE_DDR, typename Flags::ByteOrder BYTE_ORDER>
					static void init() {
						REG::REG_SPCR::And((uint8_t)~BYTE_ORDER);
						REG::REG_SPCR::Or(BYTE_ORDER);
						//Set prescalar
						REG::REG_SPSR::AndNot(SpiPrescalarData<Prescalar>::Spi2xMask);
						REG::REG_SPSR::Or(SpiPrescalarData<Prescalar>::Spi2x);
						REG::REG_SPCR::AndNot(SpiPrescalarData<Prescalar>::SPRMask);
						REG::REG_SPCR::Or(SpiPrescalarData<Prescalar>::SPR);
					}


					template <uint8_t pin, class PORT>
					static void select() {
						PORT::template BitClear<pin>();
					}
					template <uint8_t pin, class PORT>
					static void release() {
						PORT::template BitSet<pin>();
					}


					static bool writeReady() {
						return REG::REG_SPSR::template BitIsSet<REG::SpifPin>();
					}

					static bool readReady() {
						return REG::REG_SPSR::template BitIsSet<REG::SpifPin>();
					}

					static void write(uint8_t byte) {
						REG::REG_SPDR::Set(byte);
					}


					static uint8_t readMasterBlocking() {
						REG::REG_SPDR::Set(0xFF);

						while (!readReady()) { continue; }

						return REG::REG_SPDR::Get();
					}
					static uint8_t readSlaveBlocking() {
						while (!readReady()) { continue; }

						return REG::REG_SPDR::Get();
					}
					static uint8_t getByte() {
						return REG::REG_SPDR::Get();
					}

					static void deinit() {
					}

					static void enableInterrupt() {
						REG::REG_SPCR::template BitSet<REG::SpiePin>();
					}

					static void disableInterrupt() {
						REG::REG_SPCR::template BitClear<REG::SpiePin>();
					}

					static bool IsSpiInterrupt() {
						return true;
						//for compatibility
					}
					static bool isMaster() {
						return REG::template BitIsSet <REG::MstrPin>();
					}
			};

			template<class REG>
			class ProgramSpiRegActions {
					template <SpiSpeed speed>
					struct SpeedToNsDelay {
						enum {
							value = 1000 / speed
						};
					};
				public:
					typedef REG Regs;
					typedef typename REG::Flags Flags;
					static void setMaster() {
						REG::SPI_DDR::Or(REG::Mosi | REG::Sck); //as output
						REG::SPI_DDR::AndNot(REG::Miso);			//as input
						REG::SPI_DDR::template BitSet<REG::Ss>(); // SS as output
						REG::SPI_PORT::template BitSet<REG::Ss>();
					}
					/*
					static void setSlave() {
						assert(false); //NYI
					}
					*/
					static void enable() {
					}

					static void disable() {
					}

					template <typename Flags::ByteOrder BYTE_ORDER>
					static void init() {
						//TODO SetByte Order;
						//Set prescalar
					}

					template <SpiSpeed SPEED>
					static void select() {
						enum {
							ns = SpeedToNsDelay<SPEED>::value / 2
						};
						vd::delay_ns <ns> ();
						REG::SPI_PORT::template BitClear<REG::Sck>(); //tact back
						REG::SPI_PORT::template BitClear<REG::Ss>();
						vd::delay_ns < ns> ();
					}
					template <SpiSpeed SPEED>
					static void release() {
						enum {
							ns = SpeedToNsDelay<SPEED>::value / 2
						};
						vd::delay_ns <ns> ();
						REG::SPI_PORT::template BitSet<REG::Ss>();
						vd::delay_ns <ns> ();
					}


					static bool writeReady() {
						return true;
					}
					/*
					static bool readReady() {
						return REG::REG_SPSR::template BitIsSet<REG::SpifPin>();
					}
					*/
					template <SpiSpeed SPEED>
					static void write(uint8_t byte) {
						enum {
							ns = SpeedToNsDelay<SPEED>::value / 2
						};

						//TODO Order Byte
						for (u08 i = 0; i < 8; i++) {
							vd::delay_ns < ns> (); //form first part of pause

							if (byte & 0x80) { // set one bit
								REG::SPI_PORT::template BitSet<REG::Miso>();
							} else {
								REG::SPI_PORT::template BitClear<REG::Miso>();
							}

							byte <<=  1; //shift byte;
							vd::delay_ns < ns> (); // complete pause
							REG::SPI_PORT::template BitSet<REG::Sck>(); //tact front
							vd::delay_ns < ns * 2>(); // form impulse
							REG::SPI_PORT::template BitClear<REG::Sck>(); //tact back
						}
					}

					template <SpiSpeed SPEED>
					static uint8_t readMasterBlocking() {
						enum {
							ns = SpeedToNsDelay<SPEED>::value / 2
						};
						u08  byte =
						    0;         // ���������� ��� �������� � �����

						//TODO Byte order
						for (u08 i = 0; i < 8; i++) {   // ���� �� 8 ���
							vd::delay_ns < 2 * ns>(); // ��������� �����
							REG::SPI_PORT::template BitSet<REG::Sck>();          // ��������� �����
							vd::delay_ns < ns > (); // �������� ��������

							if (REG::SPI_PIN::template BitIsSet<REG::Miso>) { byte &= 0x01; } // ������ ��� � ���������� ���

							if (i != 7) { byte <<= 1; }   // ������� ����

							vd::delay_ns < ns > (); // ��������� �������
							REG::SPI_PORT::template BitClear<REG::Sck>();
						}

						return byte;            // �������, ��� ������
					}
					/*
					static uint8_t readSlaveBlocking() {

					}
					static uint8_t getByte() {
						assert(false); //NYI Slave feature
						return 1;
					}
					*/
					static void deinit() {
					}
					/*
					static void enableInterrupt() {
					}

					static void disableInterrupt() {
					}

					static bool IsSpiInterrupt() {
						return true;
						//for compatibility
					}*/
					static bool isMaster() {
						return true; //TODO
					}
			};



			typedef struct {} SpiFakeHelperStub_t;

		}
		#ifdef CPH_ENABLE_SPI
		typedef Private::SpiRegActions<Private::SpiReg> HardwareSpi0_t;
	#endif
	}
}


