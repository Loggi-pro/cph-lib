add_avr_executable(template_avr ${MCU} template_avr.cpp)
configure_cph_library(NAME cph-template_avr MCU ${MCU} COMPONENTS uart0 timer0 timer1 spi)
target_link_libraries(template_avr.elf PUBLIC void-${MCU} cph-template_avr PRIVATE project_options project_warnings)