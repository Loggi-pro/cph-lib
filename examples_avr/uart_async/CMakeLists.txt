add_avr_executable(uart_async ${MCU} uart_async.cpp)
configure_cph_library(NAME cph-uart_async MCU ${MCU} COMPONENTS uart0)
target_link_libraries(uart_async.elf PUBLIC void-${MCU} cph-uart_async PRIVATE project_options project_warnings)