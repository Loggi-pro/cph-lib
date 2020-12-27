#pragma once




namespace cph {
	enum uart_number_t {
		UART_FAKE = 255,
		#ifdef CPH_HAS_UART0
		UART_0 = 0,
		#endif
		#ifdef CPH_HAS_UART1
		UART_1 = 1,
		#endif
		#ifdef CPH_HAS_UART2
		UART_2 = 2,
		#endif
		#ifdef CPH_HAS_UART3
		UART_3 = 3,
		#endif
		#ifdef CPH_HAS_UART4
		UART_4 = 4,
		#endif
	};

	namespace Private {
		template <uart_number_t Number>
		struct UartSelector {};


		template <>
		struct UartSelector <UART_FAKE> {
			using selected = UsartFake;

		};


		#ifdef CPH_HAS_UART0
		template<>
		struct UartSelector<UART_0> {
			using selected = Usart0;
			//			using spy = Usart_Spy;
		};

		#endif
		#ifdef CPH_HAS_UART1
		template<>
		struct UartSelector<UART_1> {
			using selected = Usart1;
			using spy = Usart_Spy;
		};
		#endif
		#ifdef CPH_HAS_UART2
		template<>
		struct UartSelector<UART_2> {
			using selected = Usart2;
			using spy = Usart_Spy;
		};
		#endif
		#ifdef CPH_HAS_UART3
		template<>
		struct UartSelector<UART_3> {
			using selected = Usart3;
			using spy = Usart_Spy;
		};
		#endif
		#ifdef CPH_HAS_UART4
		template<>
		struct UartSelector<UART_4> {
			using selected = Usart4;
			using spy = Usart_Spy;
		};
		#endif
	}
}