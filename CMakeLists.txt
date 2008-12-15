##############
## Prologue ##
##############
cmake_minimum_required(VERSION 2.6)
# For ease of use later
set(PROJECT_NAME lrcon)
set(PROJECT_VERSION 0.6.1)
project(${PROJECT_NAME})

# General properties of the build
include_directories(${CMAKE_SOURCE_DIR}/include/)
enable_testing()
add_subdirectory("tests")

# For different types of CMAKE_BUILD_TYPE
# TODO: is it allowed for me to default these?
set(CMAKE_C_FLAGS_DEBUG "-g -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall")
set(CMAKE_C_FLAGS_RELEASE "-O3 -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall")

###############################
## Finding and Configuration ##
###############################

# See the FindQt4.cmake for what this does
find_package(Qt4)
include(${QT_USE_FILE})

find_package(Doxygen)
find_package(LATEX)

####################
## Building stuff ##
####################

# When cross-compiling, the right suffix doesn't get added.
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(BIN_LRCON "lrcon.exe")
  set(BIN_QRCON "qrcon.exe")
else()
  set(BIN_LRCON "lrcon")
  set(BIN_QRCON "qrcon")
endif()

set(BIN_QRCON_SRCS src/qrcon.cpp src/ServerManager.cpp)
set(BIN_QRCON_MOC_SRCS src/ServerManager.hpp)

# TODO: the dist target should really build this... it needs other work though, since
#       we need to tell cmake that we only care the Qt4 components aren't there if the
#       moc file also isn't there; also we would have to build it in a known place...
#       Use qt4_wrap_moc, but too much hassle for now I think.
qt4_wrap_cpp(BIN_QRCON_MOC_OUTPUT ${BIN_QRCON_MOC_SRCS})
add_executable(${BIN_QRCON} ${BIN_QRCON_SRCS} ${BIN_QRCON_MOC_OUTPUT})
target_link_libraries(${BIN_QRCON} ${QT_LIBRARIES})

# Lrcon binary stuff
add_executable(${BIN_LRCON} src/lrcon.cpp)

# Building documentation
# TODO: docs of include -> doc/lib/html|latex
# TODO: docs of src -> doc/progs/html|latex
# eeh... we need general targets
# 
# More detail on needed sutfuf for doxugen http://64.233.183.132/search?q=cache:jFgGH77synQJ:www.cmake.org/pipermail/cmake/2006-August/010786.html+cmake+doxygen&hl=en&ct=clnk&cd=1&client=iceweasel-a
# 
# add_doxygen_dir(${DOXYFILE} input_dirs output_dir )
# add_doxygen(${DOXYFILE} output_dir )
#
# Generate a docygen file.  So how does this one work?
# - store it in a string and cat it in add_doxygen?  Is that portable?
# - filter the file to an output as a target
# doxygen_filter_doxyfile(output_variable input_doxyfile field=value, ...)
# - generate a new file
# - problem because if the options change then this target needs to be 
#   rebuilt.  The target is dependant on input_doxyfile, and the contents of
#   the field=value array...
# doxygen_create_doxyfile(output_doxyfile input_doxyfile field=value, ...)
# 
# this should be in dist anyway, so maybe it is ok to have it unconditional...
# in fact yes; I prefer that.  Rebuilding the dox every bloody time will
# take ages.
# 
# what about the output?  It gives stupid errors that I want to ungrep.  I
# guess I need a wrapper script for this anyway so no biggie (ie, to move
# it back down to doc/; or more better releases/doc/$version/; but that
# requires changes to ... lots of things.  I can add it to bunkerprivate
# without huge problems.  (or better distributions/doc/ I suppose; main
# doc is better... or ln -s to the builddir...)
# 
# what about the latex makefile?
# 
# we could just load a template and then additional confs, and then the 
# real doxyfile is the concatination of these two... then dependancies are
# not a problem.
# 
# interface for that:
# doxygen_create_config(output input_files extra_confs)
# 

function(add_doxygen template_file doxyfile_overrides extra_commands)
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

set(DOXYFILE_DEFAULT_FILE ${CMAKE_SOURCE_DIR}/Doxyfile.default)
set(DOXYFILE_OVERRIDES_FILE ${CMAKE_SOURCE_DIR}/Doxyfile.user-overrides)

# TODO: this should probably be easier to configure with ccmake
set(
DOXYGEN_EXTRAS "EXAMPLE_PATH = ${CMAKE_SOURCE_DIR}/examples/ 
                INPUT = ${CMAKE_SOURCE_DIR}/include/ 
                PROJECT_NAME = ${PROJECT_NAME} 
                PROJECT_NUMBER = ${PROJECT_VERSION} 
                OUTPUT_DIRECTORY = ${CMAKE_BINARY_DIR}/doc/ 
                GENERATE_LATEX = yes 
                GENERATE_HTML = yes "
)

add_doxygen(
  ${DOXYFILE_DEFAULT_FILE} ${DOXYFILE_OVERRIDES_FILE} 
  ${DOXYGEN_EXTRAS}
)

##################
## Installation ##
##################

# Documentation
install(
  FILES README COPYING 
  DESTINATION doc/netz/
)

# TODO: The makedoxygen target needs teh .md5 files (for rebuilds), but we want to 
#       get rid of them for installing.  How?
# TODO: If doc/ exists, then install it.  Otherwise, don't bother.  Then elsewhere
#       we should have an if REBUILD_DOXYGEN which causes the doxygen target to run
#       with make all.

# Header files
install(FILES include/lrcon.hpp DESTINATION include/)
install(
  FILES include/lrcon/query.hpp
        include/lrcon/rcon.hpp
        include/lrcon/common.hpp
  DESTINATION include/lrcon/
)

# Binaries
install(TARGETS ${BIN_LRCON} RUNTIME DESTINATION bin)
install(TARGETS ${BIN_QRCON} RUNTIME DESTINATION bin)

#################
## CPack Stuff ##
#################

# TODO: doxygen should be in the cpack, but probably optionally... how?!
# TODO: cpack should force doxygen to build