function(set_project_options target)
   set(GCC_OPTIONS
	-gdwarf-2  
    #-fpack-struct
    -fno-split-wide-types
    -fno-tree-scev-cprop
    -fno-strict-aliasing
    -fno-threadsafe-statics
    -fno-rtti
    -funsigned-char # a few optimizations
    -funsigned-bitfields 
    -fshort-enums
    -ffunction-sections 
    -fdata-sections
    -finline-functions
    -fdiagnostics-color=always
  )
  #get_target_property(TARGET_NAME ${target} OUTPUT_NAME)
  target_compile_options(${target} INTERFACE ${GCC_OPTIONS})

endfunction()