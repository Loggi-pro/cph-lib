cmake_minimum_required(VERSION 3.16)
project(cph_library)
FIND_PACKAGE(VOID REQUIRED)  #VOID_LIB_PATH
FIND_PACKAGE(CPH REQUIRED) #CPH_LIB_PATH
include_directories($ENV{LIB})

add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)
set_project_options(project_options)
add_library(project_warnings INTERFACE)
set_project_warnings(project_warnings)
if(NOT SYSTEM_PROCESSOR)
    add_subdirectory(tests)
elseif (SYSTEM_PROCESSOR STREQUAL avr)
    add_subdirectory(examples_avr)
endif()

