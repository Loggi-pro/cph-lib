#pragma once
#include <cph/platform/stm32/core/platform_clock.h>
#include <cph/gpio.h>
#include <void/assert.h>


namespace cph {
	namespace Private {
		struct ADC1_Struct {
			inline static ADC_HandleTypeDef handle;
			inline static ADC_TypeDef* const instance = ADC1;
		};
		template <class pin>
		struct getADC {};

		template<>
		struct getADC<cph::io::Pa0> {
			typedef ADC1_Struct ADC;
			static constexpr uint8_t channel = 1;//ADC1_IN1
		};

		template<>
		struct getADC<cph::io::Pa1> {
			typedef ADC1_Struct ADC;
			static constexpr uint8_t channel = 2;//ADC1_IN2
		};

		template<>
		struct getADC<cph::io::Pa2> {
			typedef ADC1_Struct ADC;
			static constexpr uint8_t channel = 3;//ADC1_IN3
		};
		template<>
		struct getADC<cph::io::Pa3> {
			typedef ADC1_Struct ADC;
			static constexpr uint8_t channel = 4;//ADC1_IN4
		};
		template<>
		struct getADC<cph::io::Pf4> {
			typedef ADC1_Struct ADC;
			static constexpr uint8_t channel = 5;//ADC1_IN5
		};
		template<>
		struct getADC<cph::io::Pc0> {
			typedef ADC1_Struct ADC;
			static constexpr uint8_t channel = 6;//ADC1_IN6
		};
		template<>
		struct getADC<cph::io::Pc1> {
			typedef ADC1_Struct ADC;
			static constexpr uint8_t channel = 7;//ADC1_IN7
		};
		template<>
		struct getADC<cph::io::Pc2> {
			typedef ADC1_Struct ADC;
			static constexpr uint8_t channel = 8;//ADC1_IN8
		};
		template<>
		struct getADC<cph::io::Pc3> {
			typedef ADC1_Struct ADC;
			static constexpr uint8_t channel = 9;//ADC1_IN9
		};
		template<>
		struct getADC<cph::io::Pf2> {
			typedef ADC1_Struct ADC;
			static constexpr uint8_t channel = 10;//ADC1_IN10
		};


		struct InternalReferenceStruct {
			static const uint16_t value = 0x028f; // 2.56
		};




		struct PlatformAdc {

				typedef uint16_t DataT;
				// ACD internal reference voltage in fixed point Q8 format
				static const uint16_t InternalReference = InternalReferenceStruct::value;

				static const unsigned TimeoutCycles = 128 * 25 / 3;




				enum Vref {
					External,
					VCC,
					Internal
				};

				enum AdcDivider {
					Div2 = RCC_ADC12PLLCLK_DIV2,
					Div4 = RCC_ADC12PLLCLK_DIV4,
					Div8 = RCC_ADC12PLLCLK_DIV8,
					Div16 = RCC_ADC12PLLCLK_DIV16,
					Div32 = RCC_ADC12PLLCLK_DIV32,
					Div64 = RCC_ADC12PLLCLK_DIV64,
					Div128 = RCC_ADC12PLLCLK_DIV128,
					Div256 = RCC_ADC12PLLCLK_DIV256
				};

				enum ClockSource {
					MainClock = 0,
					AdcClock = 0
				};

			private:
				enum ADC_Mode {
					Mode_Regular,
					Mode_Injected //! SINGLE CONVERSION ONLY(CONTINUOUS CONVERSION DONT WORK -DONT KNOW WHY)
				};
				enum ADC_Policy {
					Policy_Blocking,
					Policy_Interrupt,
					Policy_DMA
				};

				template <class pin>
				static void _init(ADC_Mode mode, ADC_Policy policy, AdcDivider div) {
					_Policy = policy;
					auto& hadc1 = getADC<pin>::ADC::handle;
					__HAL_RCC_ADC12_CLK_ENABLE();
					__HAL_RCC_ADC12_CONFIG(div);//clock for ADC (max 14MHz --> 72/6=12MHz)
					__HAL_RCC_GPIOA_CLK_ENABLE();
					ADC_MultiModeTypeDef multimode = {0};
					/* USER CODE BEGIN ADC1_Init 1 */
					/* USER CODE END ADC1_Init 1 */
					/** Common config
					*/
					hadc1.Instance = getADC<pin>::ADC::instance;
					hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
					hadc1.Init.Resolution = ADC_RESOLUTION_10B;
					hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE; //Если каналов >1 то нужно Enable
					hadc1.Init.ContinuousConvMode = ENABLE; //Regular continous mode
					hadc1.Init.DiscontinuousConvMode = DISABLE;
					hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
					hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
					hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
					hadc1.Init.NbrOfConversion = 1; //==Количество каналов
					hadc1.Init.DMAContinuousRequests = DISABLE;
					hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV; // ADC_EOC_SEQ_CONV for continuous mode
					hadc1.Init.LowPowerAutoWait = DISABLE;
					hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
					HAL_ADC_Init(&hadc1);
					pin::SetConfiguration(pin::Port::Analog);
					pin::SetPullUp(pin::Port::NoPullUp);
					multimode.Mode = ADC_MODE_INDEPENDENT;
					HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode);

					if (mode == Mode_Regular) {
						ADC_ChannelConfTypeDef sConfig = {0};
						sConfig.Channel = getADC<pin>::channel;
						sConfig.Rank = ADC_REGULAR_RANK_1;
						sConfig.SingleDiff = ADC_SINGLE_ENDED;
						sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
						sConfig.OffsetNumber = ADC_OFFSET_NONE;
						sConfig.Offset = 0;
						HAL_ADC_ConfigChannel(&hadc1, &sConfig);
					} else if (mode == Mode_Injected) {
						ADC_InjectionConfTypeDef sConfigInjected = {0};
						sConfigInjected.InjectedChannel = getADC<pin>::channel;
						sConfigInjected.InjectedRank = ADC_INJECTED_RANK_1;
						sConfigInjected.InjectedSingleDiff = ADC_SINGLE_ENDED;
						sConfigInjected.InjectedNbrOfConversion = 1;
						sConfigInjected.InjectedSamplingTime = ADC_SAMPLETIME_19CYCLES_5;
						sConfigInjected.ExternalTrigInjecConvEdge = ADC_EXTERNALTRIGINJECCONV_EDGE_NONE;
						sConfigInjected.ExternalTrigInjecConv = ADC_INJECTED_SOFTWARE_START;
						sConfigInjected.AutoInjectedConv = DISABLE; //continuous conversion
						sConfigInjected.InjectedDiscontinuousConvMode = DISABLE;
						sConfigInjected.QueueInjectedContext = DISABLE;
						sConfigInjected.InjectedOffset = 0;
						sConfigInjected.InjectedOffsetNumber = ADC_OFFSET_NONE;
						HAL_ADCEx_InjectedConfigChannel(&hadc1, &sConfigInjected);
						//
						//  HAL_ADCEx_InjectedStart_IT(&hadc1);
						//void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef* AdcHandle)
						//uhADCxConvertedValue = HAL_ADCEx_InjectedGetValue(AdcHandle, ADC_INJECTED_RANK_1);
					}

					if (policy == Policy_Blocking) {
					} else if (policy == Policy_Interrupt) {
						HAL_NVIC_SetPriority(ADC1_2_IRQn, 0, 0);
						HAL_NVIC_EnableIRQ(ADC1_2_IRQn);
						// void ADC1_2_IRQHandler(void) {
						// HAL_ADC_IRQHandler(&hadc1);
						// }
						// uint32_t uhADCxConvertedValue;
						// volatile bool wasconversion = false;
						// void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* AdcHandle) {
						// wasconversion = true;
						// uhADCxConvertedValue = HAL_ADC_GetValue(AdcHandle);
						// }
					} else if (policy == Policy_DMA) {
						DMA_HandleTypeDef hdma_adc1;
						__HAL_RCC_DMA1_CLK_ENABLE();
						hdma_adc1.Instance = DMA1_Channel1;
						hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
						hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
						hdma_adc1.Init.MemInc = DMA_MINC_DISABLE;
						hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
						hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
						hdma_adc1.Init.Mode = DMA_CIRCULAR;
						hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
						HAL_DMA_Init(&hdma_adc1);
						__HAL_LINKDMA(&hadc1, DMA_Handle, hdma_adc1);
					}

					HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
				}
			public:




				constexpr static DataT resolutionBits() {
					return 10;
				}

				static void setClockSource(ClockSource) {
					assert(false); //NYI
				}

				static void setClockDivider(AdcDivider divider) {
					assert(false); //NYI
				}

				static void setVref(Vref ref) {
					assert(false); //NYI
				}

				static void setChannel(uint8_t channel) {
					assert(false); //NYI
				}

				inline static volatile uint32_t _ADCvalue = 0;
				inline static volatile bool _IsConversionComplete = false;
				inline static ADC_Policy _Policy = Policy_Blocking;
				template <typename pin>
				static void init(AdcDivider divider, Vref ref) {
					_init<pin>(Mode_Regular, Policy_Interrupt, divider);
				}

				static void startContinuousConversions() {
					auto& hadc1 = ADC1_Struct::handle;

					if (hadc1.Init.ContinuousConvMode != ENABLE) {
						hadc1.Init.ContinuousConvMode = ENABLE;
						HAL_ADC_Init(&hadc1);
					}

					assert(_Policy == Policy_Interrupt);
					HAL_ADC_Start_IT(&ADC1_Struct::handle);
				}

				static void enableInterrupt() {
				}

				static void startSingleConversion() {
					auto& hadc1 = ADC1_Struct::handle;

					if (hadc1.Init.ContinuousConvMode != DISABLE) {
						hadc1.Init.ContinuousConvMode = DISABLE;
						HAL_ADC_Init(&hadc1);
					}

					assert(_Policy == Policy_Interrupt);
					_IsConversionComplete = false;
					HAL_ADC_Start_IT(&ADC1_Struct::handle);
				}

				static DataT read() {
					// unsigned timeout = TimeoutCycles;
					/*
					ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 1, ADC_SampleTime_4Cycles5);
					    ADC_StartConversion(ADC1);
					    while(ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC)==RESET);
					    return ADC_GetConversionValue(ADC1);
						*/
					/*while (!resultReady() && --timeout) {
						continue;
					}

					if (timeout == 0) {
						return 0xffff;
					}
					*/
					_IsConversionComplete = false;
					return _ADCvalue;
				}

				static bool resultReady() {
					assert(_Policy == Policy_Interrupt);
					return _IsConversionComplete;
				}

				static DataT singleConversion() {
					startSingleConversion();
					return read();
				}

				static void stop() {
					assert(_Policy == Policy_Interrupt);
					HAL_ADC_Stop_IT(&ADC1_Struct::handle);
					_IsConversionComplete = false;
				}

				static void enable() {
					__HAL_RCC_ADC12_CLK_ENABLE();

					if (_Policy == Policy_DMA) {
						__HAL_RCC_DMA1_CLK_ENABLE();
					}
				}

				static void disable() {
					__HAL_RCC_ADC12_CLK_DISABLE();

					if (_Policy == Policy_DMA) {
						__HAL_RCC_DMA1_CLK_DISABLE();
					}
				}

		};

	}
}