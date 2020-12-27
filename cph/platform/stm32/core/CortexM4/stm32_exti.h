#pragma once
#include <void/static_assert.h>
#include <void/meta.h>
namespace cph {
	enum EXTI_IT_TYPE {
		EXTI_RISING,
		EXTI_FALLING,
		EXTI_BOTH
	};

	/*struct details_foreach{
		template<typename T, typename F>
		static auto _apply_func(const T& l, F f) {
			typedef decltype(vd::top(l)) element;
			typedef decltype(vd::pop(l)) tail;
			f(element{}, tail{});
		}

		template<typename F, typename T, typename ... As>
		static auto _apply_func(const vd::list<>& l, F f) {

		}

		template<typename F>
		static auto _foreach(const vd::list<>& l, F f) {

		}

		template<typename F,typename T, typename ... As>
		static auto _foreach(const vd::list<T, As...>& l, F f) {
			_apply_func(l,f);
			_foreach(vd::pop(l), f);
		}

	};


	//apply predicat to each element while return in true
		template <typename F, typename T, typename ... As>
		static auto foreach(const vd::list<T, As...>& l, F f) {
			return details_foreach::_foreach(l,f);
		}*/
	struct exti {
		private:
			typedef void(*isr_handle_t)();

			static isr_handle_t _handlers[16];

			/*
				WARNING not yet tested on pin>2
				*/
			template <typename pin>
			static void _attachInterrupt(isr_handle_t handler, EXTI_IT_TYPE type) {
				__HAL_RCC_SYSCFG_CLK_ENABLE();
				//VOID_STATIC_WARNING((pin::Number <= 3)|| (pin::Number==8), "not yet tested");
				{
					const uint32_t shift =  pin::Number % 4 * 4; //8
					const uint8_t regN = pin::Number / 4; // 0
					const uint8_t value = (pin::Port::Id - 'A'); //3
					const uint32_t mask = ~(0x0F << shift);
					volatile uint32_t temp = SYSCFG->EXTICR[regN];
					temp &= mask;
					temp |= value << shift;
					SYSCFG->EXTICR[regN] = temp;
				}
				//DisableAllInterrupt
				BIT_CLEAR(EXTI->RTSR, pin::Number);
				BIT_CLEAR(EXTI->FTSR, pin::Number);

				switch (type) {
				case EXTI_RISING:
					BIT_SET(EXTI->RTSR, pin::Number);
					break;

				case EXTI_FALLING:
					BIT_SET(EXTI->FTSR, pin::Number);
					break;

				case EXTI_BOTH:
					BIT_SET(EXTI->RTSR, pin::Number);
					BIT_SET(EXTI->FTSR, pin::Number);
					break;
				}

				_handlers[pin::Number] = handler;

// Функции CMSIS разрешающие прерывания в NVIC
				switch (pin::Number) {
				case 0:
					NVIC_EnableIRQ (EXTI0_IRQn);
					break;

				case 1:
					NVIC_EnableIRQ (EXTI1_IRQn);
					break;

				case 2:
					NVIC_EnableIRQ (EXTI2_TSC_IRQn);
					break;

				case 3:
					NVIC_EnableIRQ (EXTI3_IRQn);
					break;

				case 4:
					NVIC_EnableIRQ (EXTI4_IRQn);
					break;

				case 5:
				case 6:
				case 7:
				case 8:
				case 9:
					NVIC_EnableIRQ (EXTI9_5_IRQn);
					break;

				case 10:
				case 11:
				case 12:
				case 13:
				case 14:
				case 15:
					NVIC_EnableIRQ (EXTI15_10_IRQn);
					break;

				default:
					// NVIC_EnableIRQ (EXTI9_5_IRQn);
					break;
				}

				//Расставляем приоритеты (опционально, можно закомментить и тогда приоритеты будут равны)
//NVIC_SetPriority (EXTI2_IRQn, 2);
// Разрешаем прерывания в периферии
//	EXTI->IMR |= (EXTI_IMR_MR0 | EXTI_IMR_MR1 | EXTI_IMR_MR2);
//BIT_CLEAR(EXTI->IMR,pin::Number);
				BIT_SET(EXTI->IMR, pin::Number);
			}
		public:
			static void call(uint8_t n) {
				_handlers[n]();
			}

			template <typename pin1, typename ...pins>
			static void attachInterrupt(isr_handle_t handler,EXTI_IT_TYPE type) {
			typedef typename vd::fn::list<pin1, pins...> list;			
			auto convertToNumbers = [] (auto t) { //reset all info except number of pin
				typedef typename decltype (t)::type type;
				return vd::constInt<type::Number>{};
			};

			constexpr auto numbers = vd::fn::transform(list{},convertToNumbers);
			VOID_STATIC_ASSERT(vd::fn::is_unique(numbers)==true); //different pin numbers
			vd::fn::foreach(list{},[](auto t,isr_handle_t handler, EXTI_IT_TYPE type){
				typedef typename decltype(t)::type pin;
				_attachInterrupt<pin>(handler,type);
			},handler,type);
			}

	};

}