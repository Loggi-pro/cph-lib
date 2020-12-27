
#INPUT: ${VOID_LIB_PATH} -> path to void/platform_specific.h
#SET(CMAKE_MODULE_PATH ${CURRENT_DIR})
if (NOT EXISTS ${VOID_LIB_PATH})
	message(FATAL_ERROR "CMake error: Directory VOID_LIB_PATH for void library not specified: ${VOID_LIB_PATH}")
endif()
STRING(REGEX REPLACE "\\\\" "/" VOID_LIB_PATH ${VOID_LIB_PATH})
#ARGS: 	MCU ${MCU} or "win32" - for stm32/avr/stm8
#		NAME= library name
#		ENABLE_SAFE_INTEGERS - enable boost/safe_integers (for win32 project only)
#Global:
#	SYSTEM_PROCESSOR stm32/avr/stm8
#	VOID_LIB_PATH: path to lib
#depends from ${CHIP_HEADER_${MCU}} for avr
#out = void-${MCU}
function(configure_void_library)
	list(APPEND CMAKE_MESSAGE_CONTEXT "void")
	set(noValues ENABLE_SAFE_INTEGERS)
	set(singleValues MCU NAME)
	include(CMakeParseArguments)
	cmake_parse_arguments(ARG "${noValues}" "${singleValues}" "" ${ARGN})
	if (ARG_NAME)
		set(new_target ${ARG_NAME})
	else()
		MESSAGE(STATUS "NAME not specified, used default:  void-${ARG_MCU}")
		set(new_target void-${ARG_MCU})
	endif()

	add_library(${new_target} ${VOID_LIB_PATH}/void/src/time/dateutils.cpp)
	target_compile_features(${new_target} PRIVATE cxx_std_17)

	if (SYSTEM_PROCESSOR STREQUAL stm32)
		STM32_CHIP_GET_ARCH(${ARG_MCU} MCU_TYPE)
		LIST(APPEND DEFINES _ARM)
		LIST(APPEND DEFINES __CORTEX_${MCU_TYPE}__)
	elseif(SYSTEM_PROCESSOR STREQUAL avr)
		LIST(APPEND DEFINES _AVR)
		LIST(APPEND DEFINES ${CHIP_HEADER_${MCU}})
	elseif(SYSTEM_PROCESSOR STREQUAL stm8)
		LIST(APPEND DEFINES _STM8)
	endif()
	if (ARG_ENABLE_SAFE_INTEGERS AND ARG_MCU STREQUAL win32)
		LIST(APPEND DEFINES _VOID_SAFE_INTEGERS)
	endif()
	target_compile_definitions(${new_target}  PUBLIC ${DEFINES})
	target_compile_options (${new_target}  PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-includevoid/platform_specific.h>)
	target_include_directories(${new_target} PUBLIC ${VOID_LIB_PATH})
	MESSAGE(VERBOSE "Add sources: ${VOID_LIB_PATH}/void/src/time/dateutils.cpp")
	MESSAGE(VERBOSE "Add include directories: ${VOID_LIB_PATH}")
	MESSAGE(VERBOSE "Add force includes: void/platform_specific.h")
	MESSAGE(VERBOSE "Add defines: ${DEFINES}")
	list(POP_BACK CMAKE_MESSAGE_CONTEXT)

endfunction()

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(VOID DEFAULT_MSG)