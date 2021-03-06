GET_FILENAME_COMPONENT(CURRENT_DIR ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)
#setup path for current dir and libs dir
LIST(APPEND CMAKE_MODULE_PATH "${CURRENT_DIR}")
LIST(APPEND CMAKE_MODULE_PATH "${CURRENT_DIR}/../libs")
#run other
include(${CURRENT_DIR}/prescript.cmake)
#setup gcc compiler for win32
include(${CURRENT_DIR}/gcc.cmake)
include(${CURRENT_DIR}/compiler_flags.cmake)
include(${CURRENT_DIR}/compiler_warnings.cmake)
include(${CURRENT_DIR}/compiler_options.cmake)
#include project settings
include(${CURRENT_DIR}/project_options.cmake)
#functions(api)
include(${CURRENT_DIR}/../common/common_functions.cmake)
include(${CURRENT_DIR}/../common/Doxygen.cmake)