##########################################################################
# add_avr_executable
# - IN_VAR: EXECUTABLE_NAME
# - IN_VAR: MCU_TYPE
# Creates targets and dependencies for AVR toolchain, building an
# executable. Calls add_executable with ELF file as target name, so
# any link dependencies need to be using that target, e.g. for
# target_link_libraries(<EXECUTABLE_NAME>.elf ...).
##########################################################################

function(add_avr_executable EXECUTABLE_NAME MCU)

	if(NOT ARGN)
	  message(FATAL_ERROR "No source files given for ${EXECUTABLE_NAME}.")
	endif(NOT ARGN)
	# set file names
	set(OUTPUT_DIR ${EXECUTABLE_OUTPUT_PATH})
	set(elf_file ${EXECUTABLE_NAME}.elf)
	set(bin_file ${EXECUTABLE_NAME}.bin)
	set(hex_file ${EXECUTABLE_NAME}.hex)
	set(lst_file ${EXECUTABLE_NAME}.lst)
	set(map_file ${EXECUTABLE_NAME}.map)
	set(eeprom_image ${EXECUTABLE_NAME}.eep)

	

	#check that chip header for std lib exist
	if (NOT CHIP_HEADER_${MCU})
		message(FATAL_ERROR "Can't find header for Mcu=${MCU} in <chip_functions.cmake>. Possibly wrong mcu specified")
	endif()
	# elf file
	add_executable(${elf_file} EXCLUDE_FROM_ALL ${ARGN})
	set_target_properties(
	  ${elf_file}
	  PROPERTIES
		 COMPILE_OPTIONS "-mmcu=${MCU}"
		 COMPILE_DEFINITIONS "MCU=${MCU};${CHIP_HEADER_${MCU}}"
		 LINK_OPTIONS "-mmcu=${MCU};-Wl,--gc-sections;-mrelax;-Wl,-Map,${map_file}"
	)


	
	#=====================ADD STANDART LIBRARY=================================================
	target_compile_options (${elf_file}  PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-includeavr/io.h>)
	add_custom_command(
	  OUTPUT ${hex_file}
	  COMMAND
		 ${CMAKE_OBJCOPY} -j .text -j .data -O ihex ${OUTPUT_DIR}/${elf_file} ${OUTPUT_DIR}/${hex_file}
	  DEPENDS ${elf_file}
	)

	add_custom_command(
	  OUTPUT ${bin_file}
	  COMMAND
		 ${CMAKE_OBJCOPY} -O binary ${OUTPUT_DIR}/${elf_file} ${OUTPUT_DIR}/${bin_file}
	  DEPENDS ${elf_file}
	)

	add_custom_command(
	  OUTPUT ${lst_file}
	  COMMAND
		 ${CMAKE_OBJDUMP} -d ${OUTPUT_DIR}/${elf_file} > ${lst_file}
	  DEPENDS ${elf_file}
	)

	# eeprom
	add_custom_command(
	  OUTPUT ${eeprom_image}
	  COMMAND
		 ${CMAKE_OBJCOPY} -j .eeprom --set-section-flags=.eeprom=alloc,load
			--change-section-lma .eeprom=0 --no-change-warnings
			-O ihex ${OUTPUT_DIR}/${elf_file} ${OUTPUT_DIR}/${eeprom_image}
	  DEPENDS ${elf_file}
	)
	add_custom_target(
	  ${EXECUTABLE_NAME}
	  ALL
	  DEPENDS ${hex_file} ${lst_file} ${eeprom_image} ${bin_file} 
	)

	SET(CLEAN_FILES ${OUTPUT_DIR}/${map_file} ${OUTPUT_DIR}/${hex_file} ${OUTPUT_DIR}/${eeprom_image} ${OUTPUT_DIR}/${bin_file} ${CMAKE_CURRENT_BINARY_DIR}/${lst_file})
	get_target_property(ADDITIONAL_CLEAN_FILES ${EXECUTABLE_NAME} ADDITIONAL_CLEAN_FILES)
	set_target_properties(
	  ${EXECUTABLE_NAME}
	  PROPERTIES
		 OUTPUT_NAME "${elf_file}"
		 MCU ${MCU}
		 ADDITIONAL_CLEAN_FILES  "${ADDITIONAL_CLEAN_FILES}${CLEAN_FILES}"
	)
	
	print_size(${EXECUTABLE_NAME})

	add_custom_command(
		TARGET ${EXECUTABLE_NAME} POST_BUILD
		COMMAND echo ========== Build succeeded ==========
	)

	# disassemble
	add_custom_target(
	  disassemble_${EXECUTABLE_NAME}
	  ${CMAKE_OBJDUMP} -h -S ${OUTPUT_DIR}/${elf_file} > ${OUTPUT_DIR}/${EXECUTABLE_NAME}.lst
	  DEPENDS ${elf_file}
	)
	
	
endfunction(add_avr_executable)
