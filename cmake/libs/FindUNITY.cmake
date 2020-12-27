
#INPUT: ${VOID_LIB_PATH} -> path to void/platform_specific.h
#SET(CMAKE_MODULE_PATH ${CURRENT_DIR})
if (NOT EXISTS ${UNITY_LIB_PATH})
	message(FATAL_ERROR "CMake error: Directory UNITY_LIB_PATH for void library not specified: ${UNITY_LIB_PATH}")
endif()
STRING(REGEX REPLACE "\\\\" "/" UNITY_LIB_PATH ${UNITY_LIB_PATH})
#ARGS: ${TARGET}
# NAME
function(configure_unity_library)
	list(APPEND CMAKE_MESSAGE_CONTEXT "unity")
	set(singleValues NAME)
	include(CMakeParseArguments)
	cmake_parse_arguments(ARG "" "${singleValues}" "" ${ARGN})
	if (NOT ARG_NAME)
		MESSAGE(STATUS "NAME not specified, used default:  unity")
		set(new_target unity)
	else()
		set(new_target ${ARG_NAME})
	endif()

	add_library(${new_target} ${UNITY_LIB_PATH}/unity/unity.cpp ${UNITY_LIB_PATH}/unity/unity_assertion.cpp ${UNITY_LIB_PATH}/unity/unity_fixture.cpp)
	target_compile_definitions(${new_target} PUBLIC _UNITY)
	target_compile_features(${new_target} PRIVATE cxx_std_17)
	list(POP_BACK CMAKE_MESSAGE_CONTEXT)
endfunction()

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(UNITY DEFAULT_MSG)

