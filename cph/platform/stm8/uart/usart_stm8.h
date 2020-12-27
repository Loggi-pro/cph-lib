#pragma once
#include <cph\ioreg.h>
#include <void\static_assert.h>
#include <stm8s_uart1.h>
#include <stm8s_uart3.h>
#include <stm8s.h>
namespace cph::Private {
//TODO Remove UsartRegs::UsartFlags flags and convert functions with comparisons, use directly flags.



	#ifdef UART1
#define CPH_HAS_UART1
	struct Usart1Regs {
		enum {SYNC_MODE_SUPPORT = true};
		enum InterruptFlags_Value {
			NoInterrupt = 0,
			RxNotEmptyInt = UART1_IT_RXNE,
			TxCompleteInt = UART1_IT_TC,
			TxEmptyInt = UART1_IT_TXE,
			LineIdleInt = 	UART1_IT_IDLE,
			OverrunErrorInt = UART1_IT_OR,
			ParityErrorInt = UART1_IT_PE,
			LineBreakInt = UART1_IT_LBDF,
			RxOrOverrunInt = UART1_IT_RXNE_OR
		};
		typedef UART1_IT_TypeDef InterruptFlags_Type;

		enum Flag {
			TxComplete = UART1_FLAG_TC,
			TxEmpty = UART1_FLAG_TXE,
			RxComplete = UART1_FLAG_RXNE,
			IdleLineDetected = UART1_FLAG_IDLE,
			LineBreakDetected = UART1_FLAG_LBDF,
			SendBreak = UART1_FLAG_SBK,
		};
		typedef UART1_Flag_TypeDef Flag_Type;

		enum ParityMode_Value {
			NoneParity = UART1_PARITY_NO,
			EvenParity = UART1_PARITY_EVEN,
			OddParity = UART1_PARITY_ODD,
		};
		typedef UART1_Parity_TypeDef Parity_Type;

		enum StopBits_Value {
			OneStopBit = UART1_STOPBITS_1,
			HalfStopBit = UART1_STOPBITS_0_5,
			TwoStopBits = UART1_STOPBITS_2,
			OneAndHalfStopBits = UART1_STOPBITS_1_5,
		};

		typedef UART1_StopBits_TypeDef StopBits_Type;

		enum Mode_Value {
			Disabled = UART1_MODE_TX_DISABLE | UART1_MODE_RX_DISABLE,
			RxEnable = UART1_MODE_RX_ENABLE,
			TxEnable = UART1_MODE_TX_ENABLE,
			RxTxEnable = UART1_MODE_TXRX_ENABLE,
		};
		typedef UART1_Mode_TypeDef Mode_Type;

		enum DataBits_Value {
			DataBits8 = UART1_WORDLENGTH_8D,
			DataBits9 = UART1_WORDLENGTH_9D
		};

		typedef UART1_WordLength_TypeDef DataBits_Type;
		enum Error_Value {
			NoError = 0,
			OverrunError = UART1_FLAG_OR,
			NoiseError = UART1_FLAG_NF,
			FramingError = UART1_FLAG_FE,
			ParityError = UART1_FLAG_PE,
		};
		typedef UART1_Flag_TypeDef Error_Type;

		enum SyncMode_Value {
			ClockDisable = UART1_SYNCMODE_CLOCK_DISABLE,
			ClockEnable = UART1_SYNCMODE_CLOCK_ENABLE,
			CPolLow = UART1_SYNCMODE_CPOL_LOW,
			CPolHigh = UART1_SYNCMODE_CPOL_HIGH,
			CphaMiddle = UART1_SYNCMODE_CPHA_MIDDLE,
			CphaBegining = UART1_SYNCMODE_CPHA_BEGINING,
			LastBitDisable = UART1_SYNCMODE_LASTBIT_DISABLE,
			LastBitEnable = UART1_SYNCMODE_LASTBIT_ENABLE
		};

		typedef UART1_SyncMode_TypeDef SyncMode_Type;

		static bool isFlagSet(Flag flag) {
			return UART1_GetFlagStatus((Flag_Type) flag) == SET;
		}
		static bool isErrorSet(Error_Value err) {
			return UART1_GetFlagStatus((Error_Type) err) == SET;
		}
		static void clearInterruptBit(InterruptFlags_Value interrupt) {
			UART1_ClearITPendingBit((InterruptFlags_Type) interrupt);
		}
		static void setInterruptState(InterruptFlags_Value interrupt, bool enable) {
			FunctionalState state = enable == true ? ENABLE : DISABLE;
			UART1_ITConfig((InterruptFlags_Type)interrupt, state);
		}
		static void init(uint32_t BaudRate, UART1_WordLength_TypeDef WordLength,
		                 UART1_StopBits_TypeDef StopBits, UART1_Parity_TypeDef Parity,
		                 UART1_SyncMode_TypeDef SyncMode, UART1_Mode_TypeDef Mode) {
			UART1_Init(BaudRate, WordLength, StopBits, Parity, SyncMode, Mode);
		}

		static void deInit() {
			UART1_DeInit();
		}
		static void enable(bool state) {
			FunctionalState lstate = state == true ? ENABLE : DISABLE;
			UART1_Cmd(lstate);
		}
		static uint8_t receiveData8() {
			return UART1_ReceiveData8();
		}
		static void sendData8(uint8_t byte) {
			UART1_SendData8(byte);
		}
		static GPIO_TypeDef* const Port;
		static const GPIO_Pin_TypeDef PinTx;
		static const GPIO_Pin_TypeDef PinRx;
		enum {IT_USART_RXC = ITC_IRQ_UART1_RX, IT_USART_TXC = ITC_IRQ_UART1_TX, IT_USART_TXE = ITC_IRQ_UART1_TX};
		struct UsartFlags {
			enum InterruptFlags {
				NoInterrupt = 0,
				RxNotEmptyInt = 1 << 1,
				TxCompleteInt = 1 << 2,
				TxEmptyInt = 1 << 3,
				LineIdleInt = 	1 << 4,
				OverrunErrorInt = 1 << 5,
				ParityErrorInt = 1 << 6,
				LineBreakInt = 1 << 7,
				RxOrOverrunInt = 1 << 8,
				AllInterrupts = RxNotEmptyInt | TxCompleteInt | TxEmptyInt | LineIdleInt | OverrunErrorInt | ParityErrorInt | LineBreakInt | RxOrOverrunInt
			};
			enum Error {
				NoError = 0,
				OverrunError = 1 << 0,
				NoiseError = 1 << 2,
				FramingError = 1 << 3,
				ParityError = 1 << 4,
				ErrorMask = OverrunError | FramingError | ParityError
			};
			enum UsartMode {
				DataBits8 = 1 << 0,
				//	DataBits9 = UsartRegs::DataBits::DataBits9, //NYI Read\Write Functions
				//
				NoneParity =  1 << 2,
				EvenParity =  1 << 3,
				OddParity =   1 << 4,
				//
				Disabled   = 1 << 5,
				RxEnable   = 1 << 6,
				TxEnable   = 1 << 7,
				RxTxEnable = 1 << 8,
				//
				OneStopBit    = 1 << 9,
				HalfStopBit   = 1 << 10,
				TwoStopBits   = 1 << 11,
				OneAndHalfStopBits = 1 << 12,

				Default = RxTxEnable | DataBits8 | NoneParity | OneStopBit
			};
		};
		static Parity_Type _getParityMode(typename UsartFlags::UsartMode Parity) {
			if (Parity & UsartFlags::NoneParity) { return (Parity_Type)NoneParity; }

			if (Parity & UsartFlags::EvenParity) { return (Parity_Type)EvenParity; }

			if (Parity & UsartFlags::OddParity) { return (Parity_Type)OddParity; }

			return (Parity_Type) NoneParity;
		}
		static StopBits_Type _getStopBitsMode(typename UsartFlags::UsartMode StopBits) {
			if (StopBits & UsartFlags::OneStopBit) { return (StopBits_Type)OneStopBit; }

			if (StopBits & UsartFlags::HalfStopBit) { return (StopBits_Type)HalfStopBit; }

			if (StopBits & UsartFlags::TwoStopBits) { return (StopBits_Type)TwoStopBits; }

			if (StopBits & UsartFlags::OneAndHalfStopBits) { return (StopBits_Type)OneAndHalfStopBits; }

			return (StopBits_Type)OneStopBit;
		}
		static Mode_Type _getTransmissionsMode(typename UsartFlags::UsartMode TransmissionMode) {
			if (TransmissionMode & UsartFlags::Disabled) { return (Mode_Type)Disabled; }

			if (TransmissionMode & UsartFlags::RxEnable) { return (Mode_Type)RxEnable; }

			if (TransmissionMode & UsartFlags::TxEnable) { return (Mode_Type)TxEnable; }

			if (TransmissionMode & UsartFlags::RxTxEnable) { return (Mode_Type)RxTxEnable; }

			return (Mode_Type)RxTxEnable;
		}
		static DataBits_Type _getDataBitsMode(typename UsartFlags::UsartMode DataBitsMode) {
			if (DataBitsMode & UsartFlags::DataBits8) { return (DataBits_Type)DataBits8; }

			//if (DataBitsMode&DataBits9) return (typename DataBits_Type)DataBits9;
			return (DataBits_Type)DataBits8;
		}
		static void _setInterrupts(typename UsartFlags::InterruptFlags interrupts, bool State) {
			if (interrupts == UsartFlags::NoInterrupt) { return; }

			if (interrupts & UsartFlags::RxNotEmptyInt) { setInterruptState(RxNotEmptyInt, State); }

			if (interrupts & UsartFlags::TxCompleteInt) { setInterruptState(TxCompleteInt, State); }

			if (interrupts & UsartFlags::TxEmptyInt) { setInterruptState(TxEmptyInt, State); }

			if (interrupts & UsartFlags::LineIdleInt) { setInterruptState(LineIdleInt, State); }

			if (interrupts & UsartFlags::OverrunErrorInt) { setInterruptState(OverrunErrorInt, State); }

			if (interrupts & UsartFlags::ParityErrorInt) { setInterruptState(ParityErrorInt, State); }

			if (interrupts & UsartFlags::LineBreakInt) { setInterruptState(LineBreakInt, State); }

			if (interrupts & UsartFlags::RxOrOverrunInt) { setInterruptState(RxOrOverrunInt, State); }
		}
	};
#define CPH_IT_USART1_RXC   ITC_IRQ_UART1_RX
#define CPH_IT_USART1_TXE   ITC_IRQ_UART1_TX
#define CPH_IT_USART1_TXC   ITC_IRQ_UART1_TX
#ifdef __STM8_S003__
	GPIO_TypeDef* const Usart1Regs::Port = GPIOD;
	const GPIO_Pin_TypeDef Usart1Regs::PinTx = GPIO_PIN_5;
	const GPIO_Pin_TypeDef Usart1Regs::PinRx = GPIO_PIN_6;
#elif defined(__STM8_S007__)
	GPIO_TypeDef* const Usart1Regs::Port = GPIOA;
	const GPIO_Pin_TypeDef Usart1Regs::PinTx = GPIO_PIN_5;
	const GPIO_Pin_TypeDef Usart1Regs::PinRx = GPIO_PIN_4;
#else
	VOID_STATIC_ASSERT(false);
#endif

	#endif

	#ifdef UART3
#define CPH_HAS_UART3

	struct Usart3Regs {
		private:
			enum {NOT_SUPPORTED = 0};
		public:
			enum {SYNC_MODE_SUPPORT = false};
			enum InterruptFlags_Value {
				NoInterrupt = 0,
				RxNotEmptyInt = UART3_IT_RXNE,
				TxCompleteInt = UART3_IT_TC,
				TxEmptyInt = UART3_IT_TXE,
				LineIdleInt = 	UART3_IT_IDLE,
				OverrunErrorInt = UART3_IT_OR,
				ParityErrorInt = UART3_IT_PE,
				LineBreakInt = UART3_IT_LBDF,
				RxOrOverrunInt = UART3_IT_RXNE_OR
			};
			typedef UART3_IT_TypeDef InterruptFlags_Type;

			enum Flag {
				TxComplete = UART3_FLAG_TC,
				TxEmpty = UART3_FLAG_TXE,
				RxComplete = UART3_FLAG_RXNE,
				IdleLineDetected = UART3_FLAG_IDLE,
				LineBreakDetected = UART3_FLAG_LBDF,
				SendBreak = UART3_FLAG_SBK,
			};
			typedef UART3_Flag_TypeDef Flag_Type;

			enum ParityMode_Value {
				NoneParity = UART3_PARITY_NO,
				EvenParity = UART3_PARITY_EVEN,
				OddParity = UART3_PARITY_ODD,
			};
			typedef UART3_Parity_TypeDef Parity_Type;

			enum StopBits_Value {
				OneStopBit = UART3_STOPBITS_1,
				HalfStopBit = NOT_SUPPORTED,//UART3_STOPBITS_0_5,
				TwoStopBits = UART3_STOPBITS_2,
				OneAndHalfStopBits = NOT_SUPPORTED,//UART3_STOPBITS_1_5,
			};

			typedef UART3_StopBits_TypeDef StopBits_Type;

			enum Mode_Value {
				Disabled = UART3_MODE_TX_DISABLE | UART3_MODE_RX_DISABLE,
				RxEnable = UART3_MODE_RX_ENABLE,
				TxEnable = UART3_MODE_TX_ENABLE,
				RxTxEnable = UART3_MODE_TXRX_ENABLE,
			};
			typedef UART3_Mode_TypeDef Mode_Type;

			enum DataBits_Value {
				DataBits8 = UART3_WORDLENGTH_8D,
				DataBits9 = UART3_WORDLENGTH_9D
			};

			typedef UART3_WordLength_TypeDef DataBits_Type;
			enum Error_Value {
				NoError = 0,
				OverrunError = UART3_FLAG_OR_LHE,
				NoiseError = UART3_FLAG_NF,
				FramingError = UART3_FLAG_FE,
				ParityError = UART3_FLAG_PE,
			};
			typedef UART3_Flag_TypeDef Error_Type;

			//====NOT SUPPORTED====
			enum SyncMode_Value {
				ClockDisable = NOT_SUPPORTED,
				//ClockEnable=NOT_SUPPORTED,
				//CPolLow=NOT_SUPPORTED,
				//	CPolHigh=NOT_SUPPORTED,
				// CphaMiddle=NOT_SUPPORTED,
				// CphaBegining=NOT_SUPPORTED,
				// LastBitDisable=NOT_SUPPORTED,
				// LastBitEnable=NOT_SUPPORTED
			};
			//==END NOT SUPPORTED===
			typedef uint8_t SyncMode_Type;

			static bool isFlagSet(Flag flag) {
				return UART3_GetFlagStatus((Flag_Type) flag) == SET;
			}
			static bool isErrorSet(Error_Value err) {
				return UART3_GetFlagStatus((Error_Type) err) == SET;
			}
			static void clearInterruptBit(InterruptFlags_Value interrupt) {
				UART3_ClearITPendingBit((InterruptFlags_Type) interrupt);
			}
			static void setInterruptState(InterruptFlags_Value interrupt, bool enable) {
				FunctionalState state = enable == true ? ENABLE : DISABLE;
				UART3_ITConfig((InterruptFlags_Type)interrupt, state);
			}
			static void init(uint32_t BaudRate, UART3_WordLength_TypeDef WordLength,
			                 UART3_StopBits_TypeDef StopBits, UART3_Parity_TypeDef Parity,
			                 uint8_t SyncMode, UART3_Mode_TypeDef Mode) {
				UART3_Init(BaudRate, WordLength, StopBits, Parity, Mode);
			}

			static void deInit() {
				UART3_DeInit();
			}
			static void enable(bool state) {
				FunctionalState lstate = state == true ? ENABLE : DISABLE;
				UART3_Cmd(lstate);
			}
			static uint8_t receiveData8() {
				return UART3_ReceiveData8();
			}
			static void sendData8(uint8_t byte) {
				UART3_SendData8(byte);
			}
			static GPIO_TypeDef* const Port;
			static const GPIO_Pin_TypeDef PinTx;
			static const GPIO_Pin_TypeDef PinRx;
			enum {IT_USART_RXC = ITC_IRQ_UART3_RX, IT_USART_TXC = ITC_IRQ_UART3_TX, IT_USART_TXE = ITC_IRQ_UART3_TX};
			struct UsartFlags {
				enum InterruptFlags {
					NoInterrupt = 0,
					RxNotEmptyInt = 1 << 1,
					TxCompleteInt = 1 << 2,
					TxEmptyInt = 1 << 3,
					LineIdleInt = 	1 << 4,
					OverrunErrorInt = 1 << 5,
					ParityErrorInt = 1 << 6,
					LineBreakInt = 1 << 7,
					RxOrOverrunInt = 1 << 8,
					AllInterrupts = RxNotEmptyInt | TxCompleteInt | TxEmptyInt | LineIdleInt | OverrunErrorInt | ParityErrorInt | LineBreakInt | RxOrOverrunInt
				};
				enum Error {
					NoError = 0,
					OverrunError = 1 << 0,
					NoiseError = 1 << 2,
					FramingError = 1 << 3,
					ParityError = 1 << 4,
					ErrorMask = OverrunError | FramingError | ParityError
				};
				enum UsartMode {
					DataBits8 = 1 << 0,
					//	DataBits9 = UsartRegs::DataBits::DataBits9, //NYI Read\Write Functions
					//
					NoneParity =  1 << 2,
					EvenParity =  1 << 3,
					OddParity =   1 << 4,
					//
					Disabled   = 1 << 5,
					RxEnable   = 1 << 6,
					TxEnable   = 1 << 7,
					RxTxEnable = 1 << 8,
					//
					OneStopBit    = 1 << 9,
					TwoStopBits   = 1 << 11,

					Default = RxTxEnable | DataBits8 | NoneParity | OneStopBit
				};


			};
			static Parity_Type _getParityMode(typename UsartFlags::UsartMode Parity) {
				if (Parity & UsartFlags::NoneParity) { return (Parity_Type)NoneParity; }

				if (Parity & UsartFlags::EvenParity) { return (Parity_Type)EvenParity; }

				if (Parity & UsartFlags::OddParity) { return (Parity_Type)OddParity; }

				return (Parity_Type) NoneParity;
			}
			static StopBits_Type _getStopBitsMode(typename UsartFlags::UsartMode StopBits) {
				if (StopBits & UsartFlags::OneStopBit) { return (StopBits_Type)OneStopBit; }

				// if (StopBits&UsartFlags::HalfStopBit) return (StopBits_Type)HalfStopBit;
				if (StopBits & UsartFlags::TwoStopBits) { return (StopBits_Type)TwoStopBits; }

				// if (StopBits&UsartFlags::OneAndHalfStopBits) return (StopBits_Type)OneAndHalfStopBits;
				return (StopBits_Type)OneStopBit;
			}
			static Mode_Type _getTransmissionsMode(typename UsartFlags::UsartMode TransmissionMode) {
				if (TransmissionMode & UsartFlags::Disabled) { return (Mode_Type)Disabled; }

				if (TransmissionMode & UsartFlags::RxEnable) { return (Mode_Type)RxEnable; }

				if (TransmissionMode & UsartFlags::TxEnable) { return (Mode_Type)TxEnable; }

				if (TransmissionMode & UsartFlags::RxTxEnable) { return (Mode_Type)RxTxEnable; }

				return (Mode_Type)RxTxEnable;
			}
			static DataBits_Type _getDataBitsMode(typename UsartFlags::UsartMode DataBitsMode) {
				if (DataBitsMode & UsartFlags::DataBits8) { return (DataBits_Type)DataBits8; }

				//if (DataBitsMode&DataBits9) return (typename DataBits_Type)DataBits9;
				return (DataBits_Type)DataBits8;
			}
			static void _setInterrupts(typename UsartFlags::InterruptFlags interrupts, bool State) {
				if (interrupts == UsartFlags::NoInterrupt) { return; }

				if (interrupts & UsartFlags::RxNotEmptyInt) { setInterruptState(RxNotEmptyInt, State); }

				if (interrupts & UsartFlags::TxCompleteInt) { setInterruptState(TxCompleteInt, State); }

				if (interrupts & UsartFlags::TxEmptyInt) { setInterruptState(TxEmptyInt, State); }

				if (interrupts & UsartFlags::LineIdleInt) { setInterruptState(LineIdleInt, State); }

				if (interrupts & UsartFlags::OverrunErrorInt) { setInterruptState(OverrunErrorInt, State); }

				if (interrupts & UsartFlags::ParityErrorInt) { setInterruptState(ParityErrorInt, State); }

				if (interrupts & UsartFlags::LineBreakInt) { setInterruptState(LineBreakInt, State); }

				if (interrupts & UsartFlags::RxOrOverrunInt) { setInterruptState(RxOrOverrunInt, State); }
			}
	};

#define CPH_IT_USART3_RXC   ITC_IRQ_UART3_RX
#define CPH_IT_USART3_TXE   ITC_IRQ_UART3_TX
#define CPH_IT_USART3_TXC   ITC_IRQ_UART3_TX
#ifdef __STM8_S003__
#elif defined(__STM8_S007__)
	GPIO_TypeDef* const Usart3Regs::Port = GPIOD;
	const GPIO_Pin_TypeDef Usart3Regs::PinTx = GPIO_PIN_5;
	const GPIO_Pin_TypeDef Usart3Regs::PinRx = GPIO_PIN_6;
#else
	VOID_STATIC_ASSERT(false);
#endif

	#endif


#define MAX(x,y) ((x>y)?x:y)
#define MIN(x,y) ((x>y)?y:x)
	template <class UsartRegs>
	struct UsartRegsActions {
		public:
			typedef typename UsartRegs::UsartFlags Flags;
		private:


		public:
			static bool writeReady() {
				return UsartRegs::isFlagSet(UsartRegs::TxEmpty);
			}

			static bool readReady() {
				return !UsartRegs::isFlagSet(UsartRegs::RxComplete);
			}

			static void write(uint8_t byte) {
				//UART1_SendData9
				UsartRegs::sendData8(byte);
			}
			static uint8_t read() {
				//UART1_ReceiveData9
				return UsartRegs::receiveData8();
			}


			static void deinit() {
				UsartRegs::deInit();
			}
			//set mode of uart(data size, stop bits, parity)
			template <uint32_t BAUD>
			static void init(typename Flags::UsartMode mode) {
				GPIO_Init(UsartRegs::Port, UsartRegs::PinRx, GPIO_MODE_IN_FL_NO_IT);
				GPIO_Init(UsartRegs::Port, UsartRegs::PinTx, GPIO_MODE_OUT_PP_HIGH_SLOW);
				UsartRegs::init(BAUD, UsartRegs::_getDataBitsMode(mode), UsartRegs::_getStopBitsMode(mode),
				                UsartRegs::_getParityMode(mode), (typename UsartRegs::SyncMode_Type)UsartRegs::ClockDisable,
				                UsartRegs::_getTransmissionsMode(mode));
				UsartRegs::enable(ENABLE);
			}
			static void init(uint32_t baudrate, typename Flags::UsartMode mode) {
				GPIO_Init(UsartRegs::Port, UsartRegs::PinRx, GPIO_MODE_IN_FL_NO_IT);
				GPIO_Init(UsartRegs::Port, UsartRegs::PinTx, GPIO_MODE_OUT_PP_HIGH_SLOW);
				UsartRegs::init(baudrate, UsartRegs::_getDataBitsMode(mode), UsartRegs::_getStopBitsMode(mode),
				                UsartRegs::_getParityMode(mode), (typename UsartRegs::SyncMode_Type)UsartRegs::ClockDisable,
				                UsartRegs::_getTransmissionsMode(mode));
				UsartRegs::enable(ENABLE);
			}
			static void enableInterrupt(typename Flags::InterruptFlags interrupts) {
				UsartRegs::_setInterrupts(interrupts, true);
			}

			static void disableInterrupt(typename Flags::InterruptFlags interrupts) {
				UsartRegs::_setInterrupts(interrupts, false);
			}
			static bool IntRxHandler() {
				//if (!UsartRegs::isFlagSet(UsartRegs::RxComplete)) return false;
				//UsartRegs::clearInterruptBit(UsartRegs::RxNotEmptyInt);
				return true;
			}
			static bool IntTxEmptyHandler() {
				if (!UsartRegs::isFlagSet(UsartRegs::TxComplete)) { return false; }

				UsartRegs::clearInterruptBit(UsartRegs::TxEmptyInt);
				return true;
			}

			static bool IntTxCompleteHandler() {
				if (!UsartRegs::isFlagSet(UsartRegs::TxComplete)) { return false; }

				UsartRegs::clearInterruptBit(UsartRegs::TxCompleteInt);
				return true;
			}

			static typename Flags::Error getError() {
				uint8_t result = Flags::NoError;

				if (UsartRegs::isErrorSet(UsartRegs::OverrunError)) { result |= Flags::OverrunError; }

				if (UsartRegs::isErrorSet(UsartRegs::NoiseError)) { result |= Flags::NoiseError; }

				if (UsartRegs::isErrorSet(UsartRegs::FramingError)) { result |= Flags::FramingError; }

				if (UsartRegs::isErrorSet(UsartRegs::ParityError)) { result |= Flags::ParityError; }

				return (typename Flags::Error) result;
			}
	};
#ifdef CPH_HAS_UART1
	typedef typename cph::Private::UsartRegsActions<cph::Usart1Regs> Usart1;
#endif
#ifdef CPH_HAS_UART3
	typedef typename cph::Private::UsartRegsActions<cph::Usart3Regs> Usart3;
#endif
}

