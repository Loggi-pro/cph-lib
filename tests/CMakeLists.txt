FIND_PACKAGE(UNITY REQUIRED) #VOID_LIB_PATH
#add all tests
add_executable(tests main_tests.cpp ../cph/tests/modbus/test_modbus.cpp ../cph/tests/modbus/test_modbus_callback.cpp)
target_include_directories(tests PRIVATE ${UNITY_LIB_PATH} ${VOID_LIB_PATH})

#add all libraries
configure_void_library(NAME void-win32 MCU win32 F_CPU 1)
configure_unity_library(NAME unity)
configure_cph_library(NAME cph-win32 VOID_LIB void-win32 MCU win32 COMPONENTS timer_core modbus)

#configure void lib with test framework
target_link_libraries(void-win32 PUBLIC unity)
target_include_directories(void-win32 PUBLIC ${UNITY_LIB_PATH})

#link all together
target_link_libraries(tests PUBLIC unity void-win32 cph-win32 project_options project_warnings)
ADD_DOCUMENTATION(doc doc/Doxyfile)