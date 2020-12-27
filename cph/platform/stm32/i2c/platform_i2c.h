#pragma once
#include <cph/gpio.h>
#include<void/gsl.h>
#include <void/delay.h>

#define CPH_HAS_I2C_1
namespace cph::Private {

	template <uint32_t N>
	struct i2c_regs {};
	template<>
	struct i2c_regs<1> {
		static inline I2C_TypeDef* I2C_N = I2C1;
		static inline uint8_t ALT_FUNC_N = GPIO_AF4_I2C1;
		// static inline uint32_t ALT_FUNC_N = GPIO_AF7_USART3;
		static inline uint32_t CLOCK_TYPE =  RCC_PERIPHCLK_I2C1;
		static inline uint32_t CLOCK_SOURCE = RCC_I2C1CLKSOURCE_SYSCLK;
		static inline IRQn_Type INTERRUPT_EVENT = I2C1_EV_IRQn;
		static inline IRQn_Type INTERRUPT_ERROR = I2C1_ER_IRQn;
		// #define I2Cx_EV_IRQHandler              I2C1_EV_IRQHandler
// #define I2Cx_ER_IRQHandler              I2C1_ER_IRQHandler
		typedef cph::Clock::I2c1Clock clock;

		typedef cph::io::Pb6 pinSCL;
		typedef cph::io::Pb7 pinSDA;
	};

	template <uint32_t N>
	struct  i2c_hardware_t {
			typedef void(*cbOnRecieve_t)(const uint8_t& value);
			typedef bool(*cbOnTrancieve_t)(uint8_t* data);
		private:
			/* I2C TIMING Register define when I2C clock source is SYSCLK */
			/* I2C TIMING is calculated in case of the I2C Clock source is the SYSCLK = 72 MHz */
			/* This example use TIMING to 0x00C4092A to reach 1 MHz speed (Rise time = 26ns, Fall time = 2ns) */
			enum {
				I2C_SPEED_100k = 0x6000273C,
				I2C_SPEED_200k = 0x00505AFF,
				I2C_SPEED_1000k = 0x00C4092A

			};

			VOID_STATIC_ASSERT(F_CPU == 72000000);
			typedef i2c_regs<N> I2CREGS;
			static inline I2C_HandleTypeDef _hI2c;
			static inline cbOnRecieve_t _onDataRecieveCallback = nullptr;
			static inline  cbOnTrancieve_t _onDataTrancieveCallback = nullptr;
			static inline uint8_t _receiveBuffer = 0;
			static inline uint8_t _txBuffer = 0;
		public:
			static void IntHandlerEvent() {
				HAL_I2C_EV_IRQHandler(&_hI2c);
			}
			static void IntHandlerError() {
				HAL_I2C_EV_IRQHandler(&_hI2c);
			}
			static void init(uint32_t address) {
				// GPIO_InitTypeDef  GPIO_InitStruct;
				RCC_PeriphCLKInitTypeDef  RCC_PeriphCLKInitStruct; //TODO MAKE GOOD CLOCK CLASS
				HAL_RCCEx_GetPeriphCLKConfig(&RCC_PeriphCLKInitStruct);
				RCC_PeriphCLKInitStruct.PeriphClockSelection |= I2CREGS::CLOCK_TYPE;
				RCC_PeriphCLKInitStruct.I2c1ClockSelection = I2CREGS::CLOCK_SOURCE; //WORK ONLY FROM SYS CLOCK
				HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);
				I2CREGS::clock::Enable();//				__HAL_RCC_I2C1_CLK_ENABLE();
				//  __HAL_RCC_GPIOB_CLK_ENABLE();
				/*##-3- Configure peripheral GPIO ##########################################*/
				/* I2C TX GPIO pin configuration  */
				I2CREGS::pinSCL::Port::Enable();
				I2CREGS::pinSCL::SetConfiguration(I2CREGS::pinSCL::Port::AltFunc);
				I2CREGS::pinSCL::SetPullUp(I2CREGS::pinSCL::PullMode::PullUp);
				I2CREGS::pinSCL::SetSpeed(I2CREGS::pinSCL::Speed::Fastest);
				I2CREGS::pinSCL::AltFuncNumber(I2CREGS::ALT_FUNC_N);
				I2CREGS::pinSDA::Port::Enable();
				I2CREGS::pinSDA::SetConfiguration(I2CREGS::pinSDA::Port::AltFunc);
				I2CREGS::pinSDA::SetPullUp(I2CREGS::pinSDA::PullMode::PullUp);
				I2CREGS::pinSDA::SetSpeed(I2CREGS::pinSDA::Speed::Fastest);
				I2CREGS::pinSDA::AltFuncNumber(I2CREGS::ALT_FUNC_N);
				// GPIO_InitStruct.Pin       = GPIO_PIN_6;
				// GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
				// GPIO_InitStruct.Pull      = GPIO_PULLUP;
				// GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
				// GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
				// HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
				/* I2C RX GPIO pin configuration  */
				// GPIO_InitStruct.Pin       = GPIO_PIN_7;
				// GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
				// HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
				/*##-4- Configure the NVIC for I2C ########################################*/
				/* NVIC for I2Cx */
				HAL_NVIC_SetPriority(I2C1_ER_IRQn, 0, 1);
				HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
				HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 2);
				HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
				////
				_hI2c.Instance             = I2CREGS::I2C_N;
				_hI2c.Init.Timing          = I2C_SPEED_200k;
				_hI2c.Init.OwnAddress1     = address;
				_hI2c.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;//I2C_ADDRESSINGMODE_10BIT;
				_hI2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
				_hI2c.Init.OwnAddress2     = 0xFF;
				_hI2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
				_hI2c.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
				HAL_I2C_Init(&_hI2c);
				HAL_I2CEx_ConfigAnalogFilter(&_hI2c, I2C_ANALOGFILTER_ENABLE);
			}
			static void deinit() {
				__HAL_RCC_I2C1_FORCE_RESET();
				__HAL_RCC_I2C1_RELEASE_RESET();
				HAL_NVIC_DisableIRQ(I2C1_ER_IRQn);
				HAL_NVIC_DisableIRQ(I2C1_EV_IRQn);
			}
			static void reset() {
			}
			static void enable() {
			}
			static void disable() {
			}
			static bool isFree() {
				return HAL_I2C_GetState(&_hI2c) == HAL_I2C_STATE_READY;
			}
			static bool writeData(uint16_t address, const vd::gsl::span<uint8_t>& data) {
				if (HAL_I2C_GetState(&_hI2c) != HAL_I2C_STATE_READY) { return false; }

				HAL_I2C_Master_Transmit_IT(&_hI2c, address, data.data(), data.size());
				return true;
				// return HAL_I2C_GetState(&_hI2c)==HAL_I2C_STATE_READY;
			}
			static bool writeData(uint16_t address, uint16_t memAddress, const vd::gsl::span<uint8_t>& data) {
				if (HAL_I2C_GetState(&_hI2c) != HAL_I2C_STATE_READY) {
					return false;
				}

				HAL_I2C_Mem_Write_IT(&_hI2c, address, memAddress, 1, data.data(), data.size());
				//  vd::delay(300_ms);
				return true;
				// return HAL_I2C_GetState(&_hI2c)==HAL_I2C_STATE_READY;
			}
			/*
			static bool readByte(uint16_t ADDRESS, uint8_t* byte) {
				if (HAL_I2C_GetState(&_hI2c) != HAL_I2C_STATE_READY) { return false; }
				HAL_I2C_Master_Receive_IT(&_hI2c, (uint16_t)ADDRESS, (uint8_t*)&byte, 1);
				return true;
			}
			static bool readByte(uint16_t address, uint16_t memAddress, uint8_t* byte) {
				if (HAL_I2C_GetState(&_hI2c) != HAL_I2C_STATE_READY) { return false; }

				_txBuffer = byte;
				HAL_I2C_Mem_Read_IT(&_hI2c, address, memAddress, 1, (uint8_t*)&byte, 1);
				return true;
			}
			*/
			static bool writeByte(uint16_t ADDRESS, uint8_t byte) {
				if (HAL_I2C_GetState(&_hI2c) != HAL_I2C_STATE_READY) { return false; }

				_txBuffer = byte;
				HAL_I2C_Master_Transmit_IT(&_hI2c, (uint16_t)ADDRESS, (uint8_t*)&_txBuffer, 1);
				return true;
				// return HAL_I2C_GetState(&_hI2c)==HAL_I2C_STATE_READY;
			}

			static bool writeByte(uint16_t address, uint16_t memAddress, uint8_t byte) {
				if (HAL_I2C_GetState(&_hI2c) != HAL_I2C_STATE_READY) {
					return false;
				}

				_txBuffer = byte;
				HAL_I2C_Mem_Write_IT(&_hI2c, address, memAddress, 1, (uint8_t*)&_txBuffer, 1);
				return true;
				// return HAL_I2C_GetState(&_hI2c)==HAL_I2C_STATE_READY;
			}
			static void setOnReceive(cbOnRecieve_t onDataReceiveCallback) {
			}

			static uint32_t getError() {
				assert(false); //NYT
				return HAL_I2C_GetError(&_hI2c);
			}

	};

}