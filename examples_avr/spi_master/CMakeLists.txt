add_avr_executable(spi_master ${MCU} spi_master.cpp)
configure_cph_library(NAME cph-spi_master MCU ${MCU} COMPONENTS spi)
target_link_libraries(spi_master.elf PUBLIC void-${MCU} cph-spi_master PRIVATE project_options project_warnings)