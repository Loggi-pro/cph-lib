set(CMAKE_EXPORT_COMPILE_COMMANDS ON) #enable compile_commands.json in build dir
set(CMAKE_COLOR_MAKEFILE ON) #enable compile_commands.json inbuild dir
if (EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set(EXECUTABLE_OUTPUT_PATH  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
else()
    set(EXECUTABLE_OUTPUT_PATH  ${CMAKE_CURRENT_SOURCE_DIR}/bin) #default
endif()