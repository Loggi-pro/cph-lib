add_avr_executable(system_timer ${MCU} system_timer.cpp)
configure_cph_library(NAME cph-system_timer MCU ${MCU} COMPONENTS timer1)
target_link_libraries(system_timer.elf PUBLIC void-${MCU} cph-system_timer PRIVATE project_options project_warnings)