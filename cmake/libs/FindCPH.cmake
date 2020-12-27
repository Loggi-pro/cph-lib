#INPUT: ${CPH_LIB_PATH} -> path to cph folder
if (NOT EXISTS ${CPH_LIB_PATH})
	message(FATAL_FATAL_ERROR "CMake FATAL_ERROR: Directory CPH_LIB_PATH for cph library not specified: ${CPH_LIB_PATH}")
endif()
STRING(REGEX REPLACE "\\\\" "/" CPH_LIB_PATH ${CPH_LIB_PATH})
SET(CPH_CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR})
#CPH_LIB_PATH: path to lib
#Global:
#	SYSTEM_PROCESSOR stm32/avr/stm8

function(check_var var outvar)
	if (NOT EXISTS ${var})
		MESSAGE(FATAL_ERROR "Source file for cph lib (${var}) file not exists")
	else()
		SET(${outvar} ${var} PARENT_SCOPE)
	endif()

endfunction()

#ARG:
#	MCU
#	NAME
#	VOID_LIB = name of void target
function(configure_cph_library)
	list(APPEND CMAKE_MESSAGE_CONTEXT "cph")
	set(singleValues MCU NAME VOID_LIB)
	set(multiValues COMPONENTS)
	include(CMakeParseArguments)
	cmake_parse_arguments(ARG "" "${singleValues}" "${multiValues}" ${ARGN})

	if ((NOT SYSTEM_PROCESSOR) OR (ARG_MCU STREQUAL win32))
		SET(SYSTEM_PROCESSOR win32)
	endif()
	IF(NOT ARG_COMPONENTS)
		SET(ARG_COMPONENTS "${CPH_COMPONENTS_${SYSTEM_PROCESSOR}}")
		MESSAGE(STATUS "No CPH components selected, using all: ${CPH_COMPONENTS_${SYSTEM_PROCESSOR}}")
	ENDIF()

	if (NOT ARG_NAME)
		MESSAGE(STATUS "NAME not specified, used default:  cph-${ARG_MCU}")
		set(new_target cph-${ARG_MCU})
	else()
		set(new_target ${ARG_NAME})
	endif()
	if (NOT EXISTS ${CPH_LIB_PATH}/cph/platform/null.cpp)
		MESSAGE(FATAL_ERROR "Dummy(${CPH_LIB_PATH}/cph/platform/null.cpp) file not exists. Maybe wrong CPH_LIB_PATH?")
		return()
	endif()
	add_library(${new_target} ${CPH_LIB_PATH}/cph/platform/null.cpp)
	target_include_directories(${new_target} PUBLIC ${CPH_LIB_PATH})
	target_compile_features(${new_target} PUBLIC cxx_std_17)
	#link with void lib
	if (NOT ARG_VOID_LIB)
		MESSAGE(STATUS "VOID_LIB not specified, try to use default:  void-${ARG_MCU}")
		if (TARGET void-${ARG_MCU})
			target_link_libraries(${new_target} PUBLIC void-${ARG_MCU})
		else()
			MESSAGE(FATAL_ERROR "Void lib for cph library not found. Try to create void library with name `void-${ARG_NAME}` or pass `VOID_LIB` arg to `configure_cph_library` function.")
		endif()
	else()
		target_link_libraries(${new_target} PUBLIC ${ARG_VOID_LIB})
	endif()

	if (F_CPU)
		target_compile_definitions(${new_target} PUBLIC F_CPU=${F_CPU})
	endif()
	#
	include("${CPH_CMAKE_DIR}/cph-${SYSTEM_PROCESSOR}.cmake")
	cph_init(${new_target} ${ARG_WITH_TESTS})
	#parse all components
	FOREACH(cmp ${ARG_COMPONENTS})
		STRING(TOLOWER ${cmp} cmp_lower)
		if (NOT EXISTS "${CPH_CMAKE_DIR}/cph-${SYSTEM_PROCESSOR}.cmake")
			MESSAGE(FATAL_ERROR "CPH library is not supported by ${SYSTEM_PROCESSOR}. Create `${CPH_CMAKE_DIR}/cph-${SYSTEM_PROCESSOR}.cmake` file")
		endif()

		MESSAGE(VERBOSE "Handle component: ${cmp_lower}")
		cph_component_handler(${new_target} ${cmp_lower} ${ARG_WITH_TESTS})
	ENDFOREACH()
	get_target_property(sources ${new_target} SOURCES)
	MESSAGE(VERBOSE "Add sources: ${sources}")
	MESSAGE(VERBOSE "Add include directories: ${CPH_LIB_PATH}")
	get_target_property(defines ${new_target} COMPILE_DEFINITIONS)
	MESSAGE(VERBOSE "Add defines: ${defines}")
	list(POP_BACK CMAKE_MESSAGE_CONTEXT)
endfunction()


INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CPH DEFAULT_MSG)

