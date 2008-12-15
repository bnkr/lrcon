function(add_doxygen target_name template_file extra_commands)
  find_package(Doxygen)
  find_package(LATEX)

  # TODO: check confs has a couple of required fields defined.
  # TODO: if latex is enabled, then add the latex makefile to make doxygen - we need to 
  #       grep the latex_output and output
  # TODO: if latex, find dvi2ps and all the other crap it needs; make as well.
  # TODO: we must deal with it gracefully if if all breaks massively (have
  #         an overrides-overrides maybe?  Or use echo?)
  # TODO: set dot path if WANT_DOXYGEN_GRAPHICS (blah blah put in ccmakevars)
  #       how do you default a var?!?!

  if (UNIX)
    find_program(CAT_EXE cat REQUIRED)
  elseif(Windows)
    find_program(CAT_EXE type REQUIRED)
  else()
    message("Guessing the name of /usr/bin/cat")
    set(CAT_EXE cat)
  endif()

  # TODO: if cat_exe not found, then just generate from the doxyfile (but 
  #       also print an error)

  # TODO: obv. this is broken - need to know the outputdir first
  set(latex_directory ${CMAKE_BINARY_DIR}/doc/latex/)

  # TODO: only do this is latex is enabled
#   COMMAND make
#   WORKING_DIRECTORY ${latex_directory}
#   maybehave the current target called doxygen-generate,
#   another called doxygen-pdf, and the doxygen target depending  on doxygen-generate and
#   doxygen-pdf, but  only if latex is enabled.

  add_custom_target(
    ${target_name}
    COMMAND "( ${CAT_EXE} ${template_file}; echo ${extra_commands} ) | ${DOXYGEN_EXECUTABLE} -"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  )
    
endfunction()

