add_avr_executable(gui ${MCU} main_gui.cpp gui.cpp)
configure_cph_library(NAME cph-gui MCU ${MCU} COMPONENTS timer0)
target_link_libraries(gui.elf PUBLIC void-${MCU} cph-gui PRIVATE project_options project_warnings)
