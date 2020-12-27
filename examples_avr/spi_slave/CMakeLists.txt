add_avr_executable(spi-slave ${MCU} spi_slave.cpp)
configure_cph_library(NAME cph-spi-slave MCU ${MCU} COMPONENTS spi)
target_link_libraries(spi-slave.elf PUBLIC void-${MCU} cph-spi-slave PRIVATE project_options project_warnings)