add_avr_executable(modbus ${MCU} modbus.cpp)
configure_cph_library(NAME cph-modbus MCU ${MCU} COMPONENTS uart0 timer0 timer1 modbus)
target_link_libraries(modbus.elf PUBLIC void-${MCU} cph-modbus PRIVATE project_options project_warnings)