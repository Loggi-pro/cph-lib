#pragma once
#include <cph/ioreg.h>
#include <void/static_assert.h>
#include <void/atomic.h>
#include <cph/gpio.h>
#include "../core/platform_clock.h"
#include <void/bits.h>
namespace cph::Private {
#define CPH_HAS_UART1

#define CPH_HAS_UART3
#define CPH_HAS_UART4


	struct Usart1Regs {
		static inline USART_TypeDef* UART_N = USART1;
		static inline uint32_t ALT_FUNC_N = GPIO_AF7_USART1;
		static inline IRQn_Type INTERRUPT = USART1_IRQn;
		typedef cph::Clock::Usart1ClockSource source;
		typedef cph::Clock::Usart1Clock clock;
		typedef cph::io::Pc10 pinTx;
		typedef cph::io::Pc11 pinRx;
		static inline bool isAsync = false;


	};
	struct Usart3Regs {
		static inline USART_TypeDef* UART_N = USART3;
		static inline uint32_t ALT_FUNC_N = GPIO_AF7_USART3;
		static inline IRQn_Type INTERRUPT = USART3_IRQn;
		typedef cph::Clock::Usart3ClockSource source;
		typedef cph::Clock::Usart3Clock clock;
		typedef cph::io::Pb10 pinTx;
		typedef cph::io::Pe15 pinRx;
		static inline bool isAsync = false;
	};


	struct Usart4Regs {
		struct stub {
			enum {
				Pclk = 0
			};
			static void SelectClockSource(uint8_t a) {}
		};
		static inline USART_TypeDef* UART_N = UART4;
		static inline uint32_t ALT_FUNC_N = GPIO_AF5_UART4;
		static inline IRQn_Type INTERRUPT = UART4_IRQn;
		typedef stub  source;
		typedef cph::Clock::Uart4Clock clock;
		typedef cph::io::Pc10 pinTx;
		typedef cph::io::Pc11 pinRx;
		static inline bool isAsync = true;

	};
	struct UsartFlags {
		private:
			enum {NOT_SUPPORTED = 0};
		public:
			enum InterruptFlags {
				NoInterrupt = 0,
				RxNotEmptyInt = USART_CR1_RXNEIE,
				TxCompleteInt = USART_CR1_TCIE,
				TxEmptyInt = USART_CR1_TXEIE,
				LineIdleInt = USART_CR1_IDLEIE,
				ParityErrorInt = USART_CR1_PEIE,
				OverrunErrorInt = USART_CR3_EIE, //CR3
				FrameErrorInt = USART_CR3_EIE, //CR3
				NoiseErrorInt = USART_CR3_EIE //CR3
			};
			enum Error {

			};
			enum UsartDataBits {
				DataBits8 = UART_WORDLENGTH_8B,
				DataBits9 = UART_WORDLENGTH_9B
			};

			enum UsartParity {
				NoneParity = UART_PARITY_NONE,
				EvenParity = UART_PARITY_EVEN,
				OddParity = UART_PARITY_ODD,
			};
			enum UsartStopBits {
				OneStopBit = UART_STOPBITS_1,
				HalfStopBit = UART_STOPBITS_0_5,
				TwoStopBits = UART_STOPBITS_2,
				OneAndHalfStopBits = UART_STOPBITS_1_5,
			};

			enum UsartMode {
				Disabled = 0,
				RxEnable = UART_MODE_RX,
				TxEnable = UART_MODE_TX,
				RxTxEnable = UART_MODE_TX_RX,
			};


			/*

				enum Flag {
					TxComplete = UART3_FLAG_TC,
					TxEmpty = UART3_FLAG_TXE,
					RxComplete = UART3_FLAG_RXNE,
					IdleLineDetected = UART3_FLAG_IDLE,
					LineBreakDetected = UART3_FLAG_LBDF,
					SendBreak = UART3_FLAG_SBK,
				};

				enum Error_Value {
					NoError = 0,
					OverrunError = UART3_FLAG_OR_LHE,
					NoiseError = UART3_FLAG_NF,
					FramingError = UART3_FLAG_FE,
					ParityError = UART3_FLAG_PE,
				};
				*/
	};


	template <class UsartRegs>
	struct UsartRegsActions {
			struct UartSpyStub {};
		public:
			typedef UsartFlags Flags;
			typedef void(*cbOnRecieve_t)(const uint8_t& value);
			typedef bool(*cbOnTrancieve_t)(uint8_t* data);
			static inline cbOnRecieve_t _onDataRecieveCallback = nullptr;
			static inline  cbOnTrancieve_t _onDataTrancieveCallback = nullptr;
			using Spy = UartSpyStub;
		private:
			static inline UART_HandleTypeDef _huart;

			static void _enableInterrupt(typename Flags::InterruptFlags interrupt) {
				if (interrupt == Flags::FrameErrorInt || interrupt == Flags::NoiseErrorInt
				        || interrupt == Flags::OverrunErrorInt ) {
					vd::setBit(UsartRegs::UART_N->CR3, vd::Mask{interrupt});
				} else {
					vd::setBit(UsartRegs::UART_N->CR1, vd::Mask{interrupt});
				}
			}
			static void _disableInterrupt(typename Flags::InterruptFlags interrupt) {
				if (interrupt == Flags::FrameErrorInt || interrupt == Flags::NoiseErrorInt
				        || interrupt == Flags::OverrunErrorInt ) {
					vd::clearBit(UsartRegs::UART_N->CR3, vd::Mask{interrupt});
				} else {
					vd::clearBit(UsartRegs::UART_N->CR1, vd::Mask{interrupt});
				}

				/*
				const uint32_t CR1_MASK = Flags::RxNotEmptyInt | Flags::TxCompleteInt | Flags::TxEmptyInt |
				                          Flags::LineIdleInt | Flags::ParityErrorInt;
				const uint32_t CR3_MASK = Flags::OverrunErrorInt | Flags::FrameErrorInt | Flags::NoiseErrorInt;

				if (!BIT_CHECK_MASK(UsartRegs::UART_N->CR1, CR1_MASK)
				        && !BIT_CHECK_MASK(UsartRegs::UART_N->CR3, CR3_MASK)) {
					HAL_NVIC_DisableIRQ(UsartRegs::UART_N_IRQn);
				}*/
			}
			static void _IntRxHandler() {
				uint8_t byte = read(); //читаем чтобы сбросить бит прерывания

				if (_onDataRecieveCallback == nullptr) { return; }

				_onDataRecieveCallback(byte);
			}
			static void _IntTxEmptyHandler() {
				uint8_t byte;

				if (_onDataTrancieveCallback == nullptr
				        || !_onDataTrancieveCallback(&byte)) {
					_disableInterrupt(Flags::TxEmptyInt);
					// BIT_SET(UsartRegs::UART_N->RQR,UART_TXDATA_FLUSH_REQUEST);
					return;
				}

				UsartRegs::UART_N->TDR = byte;
			}
		public:


			/*	static bool writeReady() {
					// return _huart.TxXferCount == 0;
					return (UsartRegs::UART_N->ISR & USART_ISR_TXE) == 0;
				}

				static bool readReady() {
					// return _rxReceived;
					return UsartRegs::UART_N->ISR & USART_ISR_RXNE;
				}
			*/
			static void StartTranceive() {
				return _enableInterrupt(Flags::TxEmptyInt);
			}
			static void write(uint8_t byte) {
				UsartRegs::UART_N->TDR = byte;
				_enableInterrupt(Flags::TxEmptyInt);
			}
			static uint8_t read() {
				uint8_t byte = UsartRegs::UART_N->RDR;
				// UsartRegs::UART_N->RQR |= UART_RXDATA_FLUSH_REQUEST;
				return byte;
			}


			static void deinit() {
				HAL_NVIC_DisableIRQ(UsartRegs::UART_N_IRQn);
				UsartRegs::clock::Disable();
				_onDataRecieveCallback = nullptr;
				_onDataTrancieveCallback = nullptr;
				//TODO Deinit pins
			}
			static void enable() {
				UsartRegs::clock::Enable();
				setMode(typename Flags::UsartMode(_huart.Init.Mode));
			}
			static void disable() {
				UsartRegs::clock::Disable();
				setMode(Flags::Disabled);
			}

			//set mode of uart(data size, stop bits, parity)
			template <uint32_t BAUD, typename Flags::UsartDataBits DB, typename Flags::UsartStopBits SB, typename Flags::UsartParity P, typename Flags::UsartMode M>
			static void init() {
				init(BAUD, DB, SB, P, M);
			}
			static void init(uint32_t baudrate, typename Flags::UsartDataBits DB,
			                 typename Flags::UsartStopBits SB, typename Flags::UsartParity P, typename Flags::UsartMode M) {
				if (!UsartRegs::isAsync) {
					UsartRegs::source::SelectClockSource(UsartRegs::source::Pclk);
				}

				UsartRegs::clock::Enable();
				_huart.Instance = UsartRegs::UART_N;
				_huart.Init.BaudRate = baudrate;
				_huart.Init.WordLength = Flags::DataBits8;
				_huart.Init.StopBits = SB;
				_huart.Init.Parity = P;
				_huart.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
				_huart.Init.Mode       = M;//
				_huart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
				HAL_UART_Init(&_huart);
				// UsartRegs::pinRx::SetConfiguration(UsartRegs::pinRx::Port::Configuration::In);
				UsartRegs::pinRx::Port::Enable();
				UsartRegs::pinRx::SetConfiguration(UsartRegs::pinRx::Port::AltFunc);
				UsartRegs::pinRx::SetPullUp(UsartRegs::pinRx::Port::NoPullUp);
				UsartRegs::pinRx::SetSpeed(UsartRegs::pinRx::Speed::Fastest);
				UsartRegs::pinRx::AltFuncNumber(UsartRegs::ALT_FUNC_N);
				/*
				GPIO_InitTypeDef GPIO_InitStruct;
				GPIO_InitStruct.Pin = GPIO_PIN_10;
				GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
				GPIO_InitStruct.Pull = GPIO_NOPULL;//
				GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
				GPIO_InitStruct.Alternate = GPIO_AF7_UsartRegs::UART_N;//
				HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);*/
				UsartRegs::pinTx::Port::Enable();
				UsartRegs::pinTx::SetConfiguration(UsartRegs::pinTx::Port::AltFunc);
				UsartRegs::pinTx::SetPullUp(UsartRegs::pinTx::Port::NoPullUp);
				UsartRegs::pinTx::SetSpeed(UsartRegs::pinTx::Speed::Fastest);
				UsartRegs::pinTx::AltFuncNumber(UsartRegs::ALT_FUNC_N);
				/*
				GPIO_InitStruct.Pin = GPIO_PIN_15;
				GPIO_InitStruct.Pull = GPIO_NOPULL;//
				GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;//
				GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
				GPIO_InitStruct.Alternate = GPIO_AF7_UsartRegs::UART_N;//
				HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);*/
				// HAL_NVIC_SetPriority(UsartRegs::INTERRUPT, 0, 0);
				// HAL_NVIC_EnableIRQ(UsartRegs::INTERRUPT);
				setMode(M);
				HAL_NVIC_SetPriority(UsartRegs::INTERRUPT, 0, 0);
				_enableInterrupt(Flags::RxNotEmptyInt);
				HAL_NVIC_EnableIRQ(UsartRegs::INTERRUPT);
				uint8_t byte = read(); //читаем чтобы сбросить бит прерыванияч
				(void)byte;
				UsartRegs::UART_N->RQR  |= UART_TXDATA_FLUSH_REQUEST;
				UsartRegs::UART_N->RQR |= UART_RXDATA_FLUSH_REQUEST;
			}
			static void setMode(typename Flags::UsartMode m) {
				uint32_t tmp = UsartRegs::UART_N->CR1;
				vd::clearBit(tmp, vd::Mask{Flags::UsartMode::RxTxEnable});
				vd::setBit(tmp, vd::Mask{m});
				//BIT_CLEAR_MASK(tmp, Flags::UsartMode::RxTxEnable);
				//BIT_SET_VAL(tmp, m);
				UsartRegs::UART_N->CR1;
			}

			static typename Flags::UsartMode getMode() {
				uint32_t tmp = UsartRegs::UART_N->CR1;
				vd::clearBit(tmp, vd::Mask{~Flags::UsartMode::RxTxEnable});
				//BIT_CLEAR_MASK(tmp, ~Flags::UsartMode::RxTxEnable); //clear other than mask
				return (typename Flags::UsartMode)tmp;
			}

			static void setCallback(cbOnRecieve_t onDataRecieve,
			                        cbOnTrancieve_t  onDataTrancieve) {
				_onDataRecieveCallback = onDataRecieve;
				_onDataTrancieveCallback = onDataTrancieve;
			}

			static void IntHandler() {
				uint32_t isrflags = UsartRegs::UART_N->ISR;
				uint32_t cr1its = UsartRegs::UART_N->CR1;
				uint32_t cr3its = UsartRegs::UART_N->CR3;
				// HAL_UART_IRQHandler(&_huart);
				uint32_t errorflags = (isrflags & (uint32_t)(USART_ISR_PE | USART_ISR_FE | USART_ISR_ORE |
				                       USART_ISR_NE));

				/* If some errors occur */
				if ((errorflags)
				        && ((cr3its & USART_CR3_EIE)
				            || (cr1its & (USART_CR1_RXNEIE | USART_CR1_PEIE))) ) {
					/* UART parity error interrupt occurred -------------------------------------*/
					if (((isrflags & USART_ISR_PE) != RESET) && ((cr1its & USART_CR1_PEIE) != RESET)) {
						UsartRegs::UART_N->ICR = UART_CLEAR_PEF;
						// huart->ErrorCode |= HAL_UART_ERROR_PE;
					}

					/* UART frame error interrupt occurred --------------------------------------*/
					if (((isrflags & USART_ISR_FE) != RESET) && ((cr3its & USART_CR3_EIE) != RESET)) {
						UsartRegs::UART_N->ICR = UART_CLEAR_FEF;
						// huart->ErrorCode |= HAL_UART_ERROR_FE;
					}

					/* UART noise error interrupt occurred --------------------------------------*/
					if (((isrflags & USART_ISR_NE) != RESET) && ((cr3its & USART_CR3_EIE) != RESET)) {
						UsartRegs::UART_N->ICR = UART_CLEAR_NEF;
						// huart->ErrorCode |= HAL_UART_ERROR_NE;
					}

					/* UART Over-Run interrupt occurred -----------------------------------------*/
					if (((isrflags & USART_ISR_ORE) != RESET) &&
					        (((cr1its & USART_CR1_RXNEIE) != RESET) || ((cr3its & USART_CR3_EIE) != RESET))) {
						UsartRegs::UART_N->ICR = UART_CLEAR_OREF;
						// huart->ErrorCode |= HAL_UART_ERROR_ORE;
					}

					//return;
				} /* End if some error occurs */

				if (((isrflags & USART_ISR_WUF) != 0) && ((cr3its & USART_CR3_WUFIE) != 0)) {
					UsartRegs::UART_N->ICR = UART_CLEAR_WUF;
					return;
				}

				if (vd::isBitSet(isrflags, vd::Mask{USART_ISR_RXNE})) {
					_IntRxHandler();
					// BIT_SET_VAL(UsartRegs::UART_N->RQR,UART_RXDATA_FLUSH_REQUEST);
					return;
				} else if (vd::isBitSet(isrflags, vd::Mask{USART_ISR_TXE})) {
					//BIT_SET_VAL(UsartRegs::UART_N->RQR,UART_TXDATA_FLUSH_REQUEST);
					_IntTxEmptyHandler();
					return;
				}
			}
			static typename Flags::Error getError() {
				assert(false); //NYI
				return HAL_UART_GetError(&_huart);
			}
	};
	typedef struct {} Usart_Spy;
#ifdef CPH_HAS_UART1
	typedef UsartRegsActions<Usart1Regs> Usart1;
#endif
#ifdef CPH_HAS_UART3
	typedef UsartRegsActions<Usart3Regs> Usart3;
#endif
#ifdef CPH_HAS_UART4
	typedef UsartRegsActions<Usart4Regs> Usart4;
#endif

}

