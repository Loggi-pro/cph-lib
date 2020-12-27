add_avr_executable(pwm ${MCU} pwm.cpp)
configure_cph_library(NAME cph-pwm MCU ${MCU} COMPONENTS uart0 timer0 timer1)
target_link_libraries(pwm.elf PUBLIC void-${MCU} cph-pwm PRIVATE project_options project_warnings)