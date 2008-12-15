function(add_doxygen template_file doxyfile_overrides extra_commands)
  find_package(Doxygen)
  find_package(LATEX)

  set(doxyfile ${CMAKE_BINARY_DIR}/doc/Doxyfile)

  file(READ ${template_file} file_contents)
  file(WRITE ${doxyfile} ${file_contents})
  file(APPEND ${doxyfile} "${extra_commands}")

  # TODO: check confs has a couple of required fields defined.
  # TODO: if latex is enabled, then add the latex makefile to make doxygen - we need to 
  #       grep the latex_output and output
  # TODO: if latex, find dvi2ps and all the other crap it needs; make as well.
  # TODO: we must deal with it gracefully if if all breaks massively (have
  #         an overrides-overrides maybe?  Or use echo?)
  #         
  # arg there's a problem again  - if the user overrides contain things like 
  # latex and other stuff doesn't, everthing is broken again.  Maybe we have
  # to force options like that to be configured through the build system.  OR just
  # say inthereadme - if you make depenadncy findingstuff  break, then re-do it.
  # I should put the paths in the doxyfile  - that would probably sort it.
  # 
  # 
  # Ehh... it's easier to get the user to supply the template doxyfile
  # instead and then we always have control of it.  In fact it's even  easier to
  # just use echo.  Win has this at least; forget all this file building rubbish.
  # We can then always have exclusive control over whether latex is on (or whatever
  # else).

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

  # TODO: obv. this is broken.
  set(latex_directory ${CMAKE_BINARY_DIR}/doc/latex/)

  # TODO: only do this is latex is enabled
#   COMMAND make
#   WORKING_DIRECTORY ${latex_directory}
#   maybehave the current target called doxygen-generate,
#   another called doxygen-pdf, and the doxygen target depending  on doxygen-generate and
#   doxygen-pdf, but  only if latex is enabled.

  add_custom_target(
    doxygen
    COMMAND ${CAT_EXE} ${doxyfile} ${doxyfile_overrides} | ${DOXYGEN_EXECUTABLE} -
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  )
    
endfunction()

