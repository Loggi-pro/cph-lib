
FUNCTION(PREPEND var prefix)
   SET(listVar "")
   FOREACH(f ${ARGN})
      LIST(APPEND listVar "${prefix}${f}")
   ENDFOREACH(f)
   SET(${var} "${listVar}" PARENT_SCOPE)
ENDFUNCTION(PREPEND)

function(configure_cpptools target CPPTOOLSFILE_PATH)
	list(APPEND CMAKE_MESSAGE_CONTEXT "cpptools")
    include(FindPythonInterp)
	get_target_property(TARGET_NAME ${target} OUTPUT_NAME)
    #CURRENT_DIR=cmake/utils
    if (NOT EXISTS "${PYTHON_EXECUTABLE}"  OR NOT  EXISTS "${CURRENT_DIR}/../utils/gen-cpp-properties.py")
        return()
    endif()
	#force includes 
    #GET FULL PATH
    get_target_inherited_property(include_dirs ${TARGET_NAME} INCLUDE_DIRECTORIES)
    get_target_inherited_property(all_flags ${TARGET_NAME} COMPILE_OPTIONS)
	MESSAGE(VERBOSE "Find compile options: ${all_flags}")
    string(REGEX MATCHALL "-include\ ?[a-zA-Z0-9\\/_\\.]+" force_include_list "${all_flags}")
	#MESSAGE("force_include_list=${force_include_list}")
    foreach(HEADER ${force_include_list})
        string(REGEX REPLACE "-include\ ?" "" HEADER "${HEADER}")
		#MESSAGE(HEADER2=${HEADER})
        SET(PATH_VAR "PATH_${HEADER}") 
        #FIND_PATH(${PATH_VAR} NAMES ${HEADER} PATHS ${include_dirs})
        FIND_PATH(${PATH_VAR} NAMES ${HEADER} PATHS ${include_dirs}) #RESULT VARIABLE IS CACHED-NEEDS TO USE DIFF NAMES
        if (EXISTS ${${PATH_VAR}}/${HEADER})
            LIST(APPEND force_include_list_path  ${${PATH_VAR}}/${HEADER})
        else()
            MESSAGE(WARNING "Cmake warning: Can't find path for force include header <${HEADER}>")
        endif()
    endforeach()
	PREPEND(force_include_list_path "-f" ${force_include_list_path})
    MESSAGE(VERBOSE "Find force includes: ${force_include_list_path}")
    
    #defines
    get_target_inherited_property(compile_defines ${TARGET_NAME} COMPILE_DEFINITIONS)
	PREPEND(compile_defines  "-D" ${compile_defines})
    MESSAGE(VERBOSE "Find compile_defines: ${compile_defines}")
	#
    #include dirs
    get_target_inherited_property(include_dirs ${TARGET_NAME} INCLUDE_DIRECTORIES)
	PREPEND(include_dirs  "-i" ${include_dirs})
	MESSAGE(VERBOSE "Find include directories: ${include_dirs}")
	IF(CMAKE_EXPORT_COMPILE_COMMANDS)
		SET(COMPILE_COMMANDS ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json)
	ENDIF()	
	MESSAGE(VERBOSE "Using CPPTOOLSFILE_PATH=${CPPTOOLSFILE_PATH}")	
	
	execute_process(COMMAND ${PYTHON_EXECUTABLE} ${CURRENT_DIR}/../utils/gen-cpp-properties.py "--file=${CPPTOOLSFILE_PATH}"  ${force_include_list_path} ${compile_defines} ${include_dirs}  "-cc=${COMPILE_COMMANDS}"
					WORKING_DIRECTORY ${CURRENT_DIR}
					RESULT_VARIABLE rv
					)
	list(POP_BACK CMAKE_MESSAGE_CONTEXT)
ENDFUNCTION(configure_cpptools)