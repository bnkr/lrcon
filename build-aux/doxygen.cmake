function(add_doxygen target_name template_file directives_list)
  find_package(Doxygen)

  # TODO: check confs has a couple of required fields defined.
  # TODO: 
  # TODO: if latex, find dvi2ps and all the other crap it needs
  # TODO: we must deal with it gracefully if if all breaks massively (have
  #         an overrides-overrides maybe?  Or use echo?)
  # TODO: set dot path if WANT_DOXYGEN_GRAPHICS (blah blah put in ccmakevars)
  #       how do you default a var?!?!  Other paths are necessary 
  #       too.

  find_program(CAT_EXE cat type REQUIRED)
  
  if(NOT CAT_EXE) 
    message("Guessing the name of /usr/bin/cat")
    set(CAT_EXE cat)
  endif()
  
  set(doxyfile_name "${CMAKE_BINARY_DIR}/${target_name}-doxyfile-generated")
  set(additions_file "${CMAKE_BINARY_DIR}/${target_name}-doxyfile-forced")
  
  message("Making doxyfile out of ${doxyfile_name} and ${additions_file}")
  message("Enforced params are: ")
  
  file(WRITE ${additions_file} "")
  foreach(iter ${directives_list}) 
    message("${iter}")
    file(APPEND ${additions_file} "${iter}\n")
  endforeach()

  add_custom_command(
    OUTPUT "${CMAKE_BINARY_DIR}/${target_name}_doxyfile"
    COMMAND ${CAT_EXE} ${template_file} > ${doxyfile_name}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMAND ${CAT_EXE} ${additions_file} >> ${doxyfile_name}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    VERBATIM
  )

  add_custom_target(
    ${target_name}
    COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile_name}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS "${CMAKE_BINARY_DIR}/${target_name}_doxyfile"
    VERBATIM 
  )
    
  # TODO: obv. this is broken - need to know the outputdir first
  #   set(latex_directory ${CMAKE_BINARY_DIR}/doc/latex/) - grep from the file and/or overrides
  
  # TODO: we must test the file and the overrides for a value: overrides first,
  #   then if not forced, look in the file.
  #   
  #if(pdf)
  #   find_program(MAKE_EXE)
  #   find_package(LATEX)
  #   COMMAND make
  #   WORKING_DIRECTORY ${latex_directory}
  #   maybehave the current target called doxygen-generate,
  #   another called doxygen-pdf, and the doxygen target depending  on doxygen-generate and
  #   doxygen-pdf, but  only if latex is enabled.
  #endif()
    
endfunction()


