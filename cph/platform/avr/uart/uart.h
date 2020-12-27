#pragma once

#include <cph\ioreg.h>
#include <cph\enum.h>
#include <void\static_assert.h>
#include <void/assert.h>

namespace cph {
	namespace Private {
#if  defined(__AVR_ATmega16__) || defined(__AVR_ATmega8__)
#define CPH_HAS_UART0 true
#define CPH_INT_USART0_RXC USART_RXC_vect
#define CPH_INT_USART0_TXE USART_UDRE_vect
#define CPH_INT_USART0_TXC USART_TXC_vect

		struct Usart0Regs {
			IO_REG_WRAPPER(UDR, Udr, uint8_t);
			IO_REG_WRAPPER(UCSRA, Ucsra, uint8_t);
			IO_REG_WRAPPER(UCSRB, Ucsrb, uint8_t);
			IO_REG_WRAPPER(UCSRC, Ucsrc, uint8_t);
			IO_REG_WRAPPER(UBRRL, Ubrrl, uint8_t);
			IO_REG_WRAPPER(UBRRH, Ubrrh, uint8_t);

			enum {
				//UCSRA
				Rxc = 1 << RXC,
				Txc = 1 << TXC,
				Udre = 1 << UDRE,
				Fe = 1 << FE,
				Dor = 1 << DOR,
				Upe = 1 << PE,
				U2x = 1 << U2X,
				Mpcm = 1 << MPCM,
				//UCSRB
				Rxcie = 1 << RXCIE,
				Txcie = 1 << TXCIE,
				Udrie = 1 << UDRIE,
				Rxen = 1 << RXEN,
				Txen = 1 << TXEN,
				Ucsz2 = 1 << UCSZ2,
				Rxb8 = 1 << RXB8,
				Txb8 = 1 << TXB8,
				//UCSRC
				ClearC = (1 << URSEL),
				Umsel0 = (1 << UMSEL) | (1 << URSEL),
				Upm1 = (1 << UPM1) | (1 << URSEL),
				Upm0 = (1 << UPM0) | (1 << URSEL),
				Usbs = (1 << USBS) | (1 << URSEL),
				Ucsz1 = (1 << UCSZ1) | (1 << URSEL),
				Ucsz0 = (1 << UCSZ0) | (1 << URSEL),
				Ucpol = (1 << UCPOL) | (1 << URSEL)
			};
		};

#endif
#if defined (__AVR_ATmega32__)
#define CPH_HAS_UART0 true
#define CPH_INT_USART0_RXC USART_RXC_vect
#define CPH_INT_USART0_TXE USART_UDRE_vect
#define CPH_INT_USART0_TXC USART_TXC_vect

		struct Usart0Regs {
			IO_REG_WRAPPER(UDR, Udr, uint8_t);
			IO_REG_WRAPPER(UCSRA, Ucsra, uint8_t);
			IO_REG_WRAPPER(UCSRB, Ucsrb, uint8_t);
			IO_REG_WRAPPER(UCSRC, Ucsrc, uint8_t);
			IO_REG_WRAPPER(UBRRL, Ubrrl, uint8_t);
			IO_REG_WRAPPER(UBRRH, Ubrrh, uint8_t);

			enum {
				//UCSRA
				Rxc = 1 << RXC,
				Txc = 1 << TXC,
				Udre = 1 << UDRE,
				Fe = 1 << FE,
				Dor = 1 << DOR,
				Upe = 1 << PE,
				U2x = 1 << U2X,
				Mpcm = 1 << MPCM,
				//UCSRB
				Rxcie = 1 << RXCIE,
				Txcie = 1 << TXCIE,
				Udrie = 1 << UDRIE,
				Rxen = 1 << RXEN,
				Txen = 1 << TXEN,
				Ucsz2 = 1 << UCSZ2,
				Rxb8 = 1 << RXB8,
				Txb8 = 1 << TXB8,
				//UCSRC
				ClearC = (1 << URSEL),
				Umsel0 = (1 << UMSEL) | ClearC,
				Upm1 = (1 << UPM1) | ClearC,
				Upm0 = (1 << UPM0) | ClearC,
				Usbs = (1 << USBS) | ClearC,
				Ucsz1 = (1 << UCSZ1) | ClearC,
				Ucsz0 = (1 << UCSZ0) | ClearC,
				Ucpol = (1 << UCPOL) | ClearC
			};
		};
#endif
#if defined (__AVR_ATmega64__)
#define CPH_HAS_UART0 true
#define CPH_HAS_UART1 true
#define CPH_INT_USART0_RXC USART0_RX_vect
#define CPH_INT_USART0_TXE USART0_UDRE_vect
#define CPH_INT_USART0_TXC USART0_TX_vect

#define CPH_INT_USART1_RXC USART1_RX_vect
#define CPH_INT_USART1_TXE USART1_UDRE_vect
#define CPH_INT_USART1_TXC USART1_TX_vect



		struct Usart0Regs {
			IO_REG_WRAPPER(UDR0, Udr, uint8_t);
			IO_REG_WRAPPER(UCSR0A, Ucsra, uint8_t);
			IO_REG_WRAPPER(UCSR0B, Ucsrb, uint8_t);
			IO_REG_WRAPPER(UCSR0C, Ucsrc, uint8_t);
			IO_REG_WRAPPER(UBRR0L, Ubrrl, uint8_t);
			IO_REG_WRAPPER(UBRR0H, Ubrrh, uint8_t);

			enum {
				//UCSRA
				Rxc = 1 << RXC,
				Txc = 1 << TXC,
				Udre = 1 << UDRE,
				Fe = 1 << FE,
				Dor = 1 << DOR,
				Upe = 1 << UPE,
				U2x = 1 << U2X,
				Mpcm = 1 << MPCM,
				//UCSRB
				Rxcie = 1 << RXCIE,
				Txcie = 1 << TXCIE,
				Udrie = 1 << UDRIE,
				Rxen = 1 << RXEN,
				Txen = 1 << TXEN,
				Ucsz2 = 1 << UCSZ2,
				Rxb8 = 1 << RXB8,
				Txb8 = 1 << TXB8,
				//UCSRC
				ClearC = 0, //URSEL BIT not exist in atmega 64
				Umsel0 = (1 << UMSEL) | ClearC,
				Upm1 = (1 << UPM1) | ClearC,
				Upm0 = (1 << UPM0) | ClearC,
				Usbs = (1 << USBS) | ClearC,
				Ucsz1 = (1 << UCSZ1) | ClearC,
				Ucsz0 = (1 << UCSZ0) | ClearC,
				Ucpol = (1 << UCPOL) | ClearC
			};
		};


		struct Usart1Regs {
			IO_REG_WRAPPER(UDR1, Udr, uint8_t);
			IO_REG_WRAPPER(UCSR1A, Ucsra, uint8_t);
			IO_REG_WRAPPER(UCSR1B, Ucsrb, uint8_t);
			IO_REG_WRAPPER(UCSR1C, Ucsrc, uint8_t);
			IO_REG_WRAPPER(UBRR1L, Ubrrl, uint8_t);
			IO_REG_WRAPPER(UBRR1H, Ubrrh, uint8_t);

			enum {
				//UCSRA
				Rxc = 1 << RXC,
				Txc = 1 << TXC,
				Udre = 1 << UDRE,
				Fe = 1 << FE,
				Dor = 1 << DOR,
				Upe = 1 << UPE,
				U2x = 1 << U2X,
				Mpcm = 1 << MPCM,
				//UCSRB
				Rxcie = 1 << RXCIE,
				Txcie = 1 << TXCIE,
				Udrie = 1 << UDRIE,
				Rxen = 1 << RXEN,
				Txen = 1 << TXEN,
				Ucsz2 = 1 << UCSZ2,
				Rxb8 = 1 << RXB8,
				Txb8 = 1 << TXB8,
				//UCSRC
				ClearC = 0, //URSEL BIT not exist in atmega 64
				Umsel0 = (1 << UMSEL) | ClearC,
				Upm1 = (1 << UPM1) | ClearC,
				Upm0 = (1 << UPM0) | ClearC,
				Usbs = (1 << USBS) | ClearC,
				Ucsz1 = (1 << UCSZ1) | ClearC,
				Ucsz0 = (1 << UCSZ0) | ClearC,
				Ucpol = (1 << UCPOL) | ClearC
			};
		};




#endif
#if defined (__AVR_ATmega649__)
#define CPH_HAS_UART0 true
#define CPH_INT_USART0_RXC USART0_RX_vect
#define CPH_INT_USART0_TXE USART0_UDRE_vect
#define CPH_INT_USART0_TXC USART0_TX_vect



		struct Usart0Regs {
			IO_REG_WRAPPER(UDR0, Udr, uint8_t);
			IO_REG_WRAPPER(UCSR0A, Ucsra, uint8_t);
			IO_REG_WRAPPER(UCSR0B, Ucsrb, uint8_t);
			IO_REG_WRAPPER(UCSR0C, Ucsrc, uint8_t);
			IO_REG_WRAPPER(UBRR0L, Ubrrl, uint8_t);
			IO_REG_WRAPPER(UBRR0H, Ubrrh, uint8_t);

			enum {
				//UCSRA
				Rxc = 1 << RXC0,
				Txc = 1 << TXC0,
				Udre = 1 << UDRE0,
				Fe = 1 << FE0,
				Dor = 1 << DOR0,
				Upe = 1 << UPE0,
				U2x = 1 << U2X0,
				Mpcm = 1 << MPCM0,
				//UCSRB
				Rxcie = 1 << RXCIE0,
				Txcie = 1 << TXCIE0,
				Udrie = 1 << UDRIE0,
				Rxen = 1 << RXEN0,
				Txen = 1 << TXEN0,
				Ucsz2 = 1 << UCSZ02,
				Rxb8 = 1 << RXB80,
				Txb8 = 1 << TXB80,
				//UCSRC
				ClearC = 0, //URSEL BIT not exist in atmega 64
				Umsel0 = (1 << UMSEL0) | ClearC,
				Upm1 = (1 << UPM01) | ClearC,
				Upm0 = (1 << UPM00) | ClearC,
				Usbs = (1 << USBS0) | ClearC,
				Ucsz1 = (1 << UCSZ01) | ClearC,
				Ucsz0 = (1 << UCSZ00) | ClearC,
				Ucpol = (1 << UCPOL0) | ClearC
			};
		};


#endif


#if defined(__AVR_ATmega168P__) || defined(__AVR_ATmega328P__)
	#define CPH_INT_USART0_RXC USART_RX_vect
	#define CPH_INT_USART0_TXE USART_UDRE_vect
	#define CPH_INT_USART0_TXC USART_TX_vect
	#define CPH_HAS_UART0 true

		struct Usart0Regs {
			IO_REG_WRAPPER(UDR0, Udr, uint8_t);

			IO_REG_WRAPPER(UCSR0A, Ucsra, uint8_t);

			IO_REG_WRAPPER(UCSR0B, Ucsrb, uint8_t);

			IO_REG_WRAPPER(UCSR0C, Ucsrc, uint8_t);

			IO_REG_WRAPPER(UBRR0L, Ubrrl, uint8_t);

			IO_REG_WRAPPER(UBRR0H, Ubrrh, uint8_t);

			enum {
				Rxc = 1 << RXC0,
				Txc = 1 << TXC0,
				Udre = 1 << UDRE0,
				Fe = 1 << FE0,
				Dor = 1 << DOR0,
				Upe = 1 << UPE0,
				U2x = 1 << U2X0,
				Mpcm = 1 << MPCM0,

				Rxcie = 1 << RXCIE0,
				Txcie = 1 << TXCIE0,
				Udrie = 1 << UDRIE0,
				Rxen = 1 << RXEN0,
				Txen = 1 << TXEN0,
				Ucsz2 = 1 << UCSZ02,
				Rxb8 = 1 << RXB80,
				Txb8 = 1 << TXB80,

				ClearC = 0,
				Umsel1 = (1 << UMSEL01),
				Umsel0 = (1 << UMSEL00),
				Upm1 = (1 << UPM01),
				Upm0 = (1 << UPM00),
				Usbs = (1 << USBS0),
				Ucsz1 = (1 << UCSZ01),
				Ucsz0 = (1 << UCSZ00),
				Ucpol = (1 << UCPOL0)
			};
		};


#endif


#ifdef __AVR_ATmega644PA__
#define CPH_HAS_UART0 true
#define UART0_BAUD_HIGH	UBRR0H
#define UART0_BAUD_LOW	UBRR0L

#define UART0_STATUS		UCSR0A
#define UART0_CTRL	UCSR0B
#define UART0_CTRL2	UCSR0C
#define UART0_TXREADY_PIN	UDRE0
#define UART0_RXREADY_PIN	RXC0
#define UART0_DOUBLE_PIN	U2X0

#define UART0_RX_ENABLE_PIN RXEN0
#define UART0_EMPTY_PIN UDRE0
#define UART0_TX_ENABLE_PIN TXEN0
#define UART0_RX_ISR_PIN RXCIE0
#define UART0_TX_ISR_PIN RXCIE0
#define UART0_UDRIE_ISR_PIN UDRIE0

#define UART0_DATA	UDR0
#define UART0_PACKAGE_OPTIONS_8bit_woPARITY (1<<UCSZ00)|(1<<UCSZ01);
#endif


		template<class UsartNRegs>
		struct UsartFlags {
			enum InterruptFlags {
				NoInterrupt = 0,
				RxNotEmptyInt = UsartNRegs::Rxcie,
				TxCompleteInt = UsartNRegs::Txcie,
				TxEmptyInt = UsartNRegs::Udrie,
				AllInterrupts = RxNotEmptyInt | TxCompleteInt | TxEmptyInt
			};
			enum Error {
				NoError = 0,
				OverrunError = UsartNRegs::Dor,
				NoiseError = 0,
				FramingError = UsartNRegs::Fe,
				ParityError = UsartNRegs::Upe,
				ErrorMask = OverrunError | FramingError | ParityError
			};

			enum UsartDataBits {
				DataBits5 = 0,
				DataBits6 = UsartNRegs::Ucsz0 << 8,
				DataBits7 = UsartNRegs::Ucsz1 << 8,
				DataBits8 = (UsartNRegs::Ucsz1 << 8) | (UsartNRegs::Ucsz0 << 8),
				DataBits9 = UsartNRegs::Ucsz2 | (UsartNRegs::Ucsz1 << 8) | (UsartNRegs::Ucsz0 << 8),
			};

			enum UsartParity {
				NoneParity = 0,
				EvenParity = UsartNRegs::Upm1 << 8,
				OddParity = (UsartNRegs::Upm0 << 8) | (UsartNRegs::Upm1 << 8),
			};

			enum UsartMode {
				Disabled = 0,
				RxEnable = UsartNRegs::Rxen,
				TxEnable = UsartNRegs::Txen,
				RxTxEnable = RxEnable | TxEnable,
			};

			enum UsartStopBits {
				OneStopBit = 0,
				HalfStopBit = 0,
				TwoStopBits = UsartNRegs::Usbs << 8,
				OneAndHalfStopBits = UsartNRegs::Usbs << 8,
			};


		};

		DECLARE_ENUM_OPERATIONS(typename UsartFlags<Private::Usart0Regs>::InterruptFlags)

		DECLARE_ENUM_OPERATIONS(typename UsartFlags<Private::Usart0Regs>::Error)

		DECLARE_ENUM_OPERATIONS(typename UsartFlags<Private::Usart0Regs>::UsartDataBits)

		DECLARE_ENUM_OPERATIONS(typename UsartFlags<Private::Usart0Regs>::UsartParity)

		DECLARE_ENUM_OPERATIONS(typename UsartFlags<Private::Usart0Regs>::UsartMode)

		DECLARE_ENUM_OPERATIONS(typename UsartFlags<Private::Usart0Regs>::UsartStopBits)

#define MAX(x, y) ((x>y)?x:y)
#define MIN(x, y) ((x>y)?y:x)

		template<class UsartNRegs>
		struct UsartRegsActions {
			struct SpyStub {};
		public:
			typedef void(*cbOnRecieve_t)(const uint8_t &value);

			typedef bool(*cbOnTrancieve_t)(uint8_t *data);

			typedef SpyStub Spy;
		private:
			static cbOnRecieve_t _onDataRecieveCallback;
			static cbOnTrancieve_t _onDataTrancieveCallback;

			static void _setBaudrate(uint32_t baud) {
				const uint32_t clock = F_CPU;
				const uint16_t ubrr = (clock / 16 / baud - 1);
				const uint16_t ubrr2x = (clock / 8 / baud - 1);
				const uint16_t rbaund = (clock / 16 / (ubrr + 1));
				const uint16_t rbaund2x = (clock / 8 / (ubrr2x + 1));
				const uint16_t err1 = (MAX(baud, rbaund) - MIN(baud, rbaund)) * 1000 / MAX(baud, rbaund);
				const uint16_t err2 = (MAX(baud, rbaund2x) - MIN(baud, rbaund2x)) * 1000 / MAX(baud, rbaund2x);
				assert(err2 < 25 || err1 < 25);        // 2.5 % baud rate error tolerance

				//set hardware
				if (err1 > err2) {
					UsartNRegs::Ucsra::Set(UsartNRegs::U2x);
					UsartNRegs::Ubrrl::Set(ubrr2x);
					UsartNRegs::Ubrrh::Set(ubrr2x >> 8);
				} else {
					UsartNRegs::Ucsra::Set(0);
					UsartNRegs::Ubrrl::Set(ubrr);
					UsartNRegs::Ubrrh::Set(ubrr >> 8);
				}
			}

			template<uint32_t BAUD>
			static void _setBaudrate() {
				const uint32_t clock = F_CPU;
				const uint16_t ubrr = (clock / 16 / BAUD - 1);
				const uint16_t ubrr2x = (clock / 8 / BAUD - 1);
				const uint16_t rbaund = (clock / 16 / (ubrr + 1));
				const uint16_t rbaund2x = (clock / 8 / (ubrr2x + 1));
				const uint16_t err1 = (MAX(BAUD, rbaund) - MIN(BAUD, rbaund)) * 1000 / MAX(BAUD, rbaund);
				/*
						if (BAUD > rbaund) {
							err1 = (BAUD - rbaund) * 1000 / BAUD;
						} else {
							err1 = (rbaund - BAUD) * 1000 / rbaund;
						}*/
				const uint16_t err2 = (MAX(BAUD, rbaund2x) - MIN(BAUD, rbaund2x)) * 1000 / MAX(BAUD, rbaund2x);
				/*
						if (BAUD > rbaund2x) {
							err2 = (BAUD - rbaund2x) * 1000 / BAUD;
						} else {
							err2 = (rbaund2x - BAUD) * 1000 / rbaund2x;
						}*/
				VOID_STATIC_ASSERT(err2 < 25 || err1 < 25);        // 2.5 % baud rate error tolerance

				//set hardware
				if (err1 > err2) {
					UsartNRegs::Ucsra::Set(UsartNRegs::U2x);
					UsartNRegs::Ubrrl::Set(ubrr2x);
					UsartNRegs::Ubrrh::Set(ubrr2x >> 8);
				} else {
					UsartNRegs::Ucsra::Set(0);
					UsartNRegs::Ubrrl::Set(ubrr);
					UsartNRegs::Ubrrh::Set(ubrr >> 8);
				}
			}

		public:
			typedef UsartNRegs Regs;
			typedef UsartFlags<UsartNRegs> Flags;

			static void setMode(typename Flags::UsartMode m) {
				UsartNRegs::Ucsrb::AndNot(Flags::UsartMode::RxTxEnable);
				UsartNRegs::Ucsrb::Or(m);
			}

			static typename Flags::UsartMode getMode() {
				uint8_t tmp = UsartNRegs::Ucsrb::Get();
				BIT_CLEAR_MASK(tmp, ~Flags::UsartMode::RxTxEnable); //clear other than mask
				return (typename Flags::UsartMode) tmp;
			}

			static bool writeReady() {
				return (UsartNRegs::Ucsra::Get() & UsartNRegs::Udre);
			}

			static bool readReady() {
				return (UsartNRegs::Ucsra::Get() & UsartNRegs::Rxc);
			}

			static void write(uint8_t byte) {
				UsartNRegs::Udr::Set(byte);
			}

			static uint8_t read() {
				volatile uint8_t a;

				while (readReady()) {
					a = UsartNRegs::Udr::Get();
				}

				return a;
			}

			static void deinit() {
				UsartNRegs::Ucsra::Set(0);
				UsartNRegs::Ucsrb::Set(0);
				UsartNRegs::Ucsrc::Set(0);
				UsartNRegs::Ubrrl::Set(0);
				UsartNRegs::Ubrrh::Set(0);
			}

			//set mode of uart(data size, stop bits, parity,mode)
			template<uint32_t BAUD,
					typename Flags::UsartDataBits DB,
					typename Flags::UsartStopBits SB,
					typename Flags::UsartParity P,
					typename Flags::UsartMode M
			>
			static void init() {
				_setBaudrate<BAUD>();
				UsartNRegs::Ucsrb::Set(0x00);
				UsartNRegs::Ucsrc::Set(UsartNRegs::ClearC);
				uint16_t mode = DB | SB | P | M;
				UsartNRegs::Ucsrc::Set(vd::hiByte(mode));
				UsartNRegs::Ucsrb::Set(vd::lowByte(mode));
			}

			static void init(uint32_t baudrate,
							 typename Flags::UsartDataBits db,
							 typename Flags::UsartStopBits sb,
							 typename Flags::UsartParity p,
							 typename Flags::UsartMode m) {
				_setBaudrate(baudrate);
				UsartNRegs::Ucsrb::Set(0x00);
				UsartNRegs::Ucsrc::Set(UsartNRegs::ClearC);
				uint16_t mode = db | sb | p | m;
				UsartNRegs::Ucsrc::Set(vd::hiByte(mode));
				UsartNRegs::Ucsrb::Set(vd::lowByte(mode));
			}


			static void enableInterrupt(typename UsartFlags<UsartNRegs>::InterruptFlags interrupts) {
				UsartNRegs::Ucsrb::Or((uint8_t) interrupts);
			}

			static void disableInterrupt(typename UsartFlags<UsartNRegs>::InterruptFlags interrupts) {
				UsartNRegs::Ucsrb::And((uint8_t) ~interrupts);
			}

			static typename UsartFlags<UsartNRegs>::Error getError() {
				return (typename UsartFlags<UsartNRegs>::Error) (UsartNRegs::Ucsra::Get() &
																 UsartFlags<UsartNRegs>::ErrorMask);
			}


			static void IntRxHandler() {
				uint8_t value = read();

				if (_onDataRecieveCallback != nullptr) {
					_onDataRecieveCallback(value);
				};
			}

			static void IntTxEmptyHandler() {
				uint8_t value;
				bool isDataExist = false;

				if (_onDataTrancieveCallback != nullptr) {
					isDataExist = _onDataTrancieveCallback(&value);
				};

				if (isDataExist) {
					write(value);
				} else {
					disableInterrupt(Flags::TxEmptyInt);
				}

				return;
			}

			static void StartTranceive() {
				return enableInterrupt(UsartFlags<UsartNRegs>::TxEmptyInt);
			}

			static void setCallback(cbOnRecieve_t onDataRecieve,
									cbOnTrancieve_t onDataTrancieve) {
				_onDataRecieveCallback = onDataRecieve;
				_onDataTrancieveCallback = onDataTrancieve;
			}


		};

		template<class UsartNRegs>
		typename UsartRegsActions<UsartNRegs>::cbOnRecieve_t
				UsartRegsActions<UsartNRegs>::_onDataRecieveCallback = nullptr;

		template<class UsartNRegs>
		typename UsartRegsActions<UsartNRegs>::cbOnTrancieve_t
				UsartRegsActions<UsartNRegs>::_onDataTrancieveCallback = nullptr;


#undef MAX
#undef MIN
	}
#if defined(CPH_HAS_UART0) & defined(CPH_ENABLE_UART0)
		typedef typename cph::Private::UsartRegsActions<cph::Private::Usart0Regs> Usart0;
#endif
#if defined(CPH_HAS_UART1) & defined(CPH_ENABLE_UART1)
		typedef typename cph::Private::UsartRegsActions<cph::Private::Usart1Regs> Usart1;
#endif
#if defined(CPH_HAS_UART2) & defined(CPH_ENABLE_UART2)
		typedef typename cph::Private::UsartRegsActions<cph::Private::Usart2Regs> Usart2;
#endif
#if defined(CPH_HAS_UART3) & defined(CPH_ENABLE_UART3)
		typedef typename cph::Private::UsartRegsActions<cph::Private::Usart3Regs> Usart3;
#endif

}

