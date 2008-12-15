##############
## Prologue ##
##############
cmake_minimum_required(VERSION 2.6)
# For ease of use later
set(PROJECT_NAME lrcon)
set(PROJECT_VERSION 0.6.1)
project(${PROJECT_NAME})
include("build-aux/doxygen.cmake")

# General properties of the build
include_directories(${CMAKE_SOURCE_DIR}/include/)
enable_testing()
add_subdirectory("tests")

# For different types of CMAKE_BUILD_TYPE
# TODO: it seems that ccmake just overwrites this stuff - 
#       how can I put a default in there?
set(CMAKE_C_FLAGS_DEBUG "-g -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall")
set(CMAKE_C_FLAGS_RELEASE "-O3 -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall")

# Global consts for libraries etc.
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(LRCON_LIBRARIES -lws2_32)
else()
  set(LRCON_LIBRARIES "")
endif()
  

###########################
## Building normal stuff ##
##########################

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows" AND ${CMAKE_CXX_COMPILER} MATCHES mingw)
  set(BIN_LRCON "lrcon.exe")
else()
  set(BIN_LRCON "lrcon")
endif()

# Lrcon binary stuff
add_executable(${BIN_LRCON} src/lrcon.cpp )
target_link_libraries(${BIN_LRCON} ${LRCON_LIBRARIES})

####################
## Building Qrcon ##
####################

# TODO: add QT_USE_CMAKE_FINDER to advanced vars; 
# TODO: put a docstring on it somehow.

if(QT_USE_CMAKE_FINDER)
  find_package(Qt4)
  include(${QT_USE_FILE})
else()
  # TODO: patch FindQt4.cmake to take a user-defined spec file, if necessary
  # TODO: add QT_QMAKE_SPEC_FILE

  include("build-aux/FindQt4.cmake")
  include(${QT_USE_FILE})
endif()

# When cross-compiling, the right suffix doesn't get added.
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows" AND ${CMAKE_CXX_COMPILER} MATCHES mingw)
  set(BIN_QRCON "qrcon.exe")
else()
  set(BIN_QRCON "qrcon")
endif()

set(BIN_QRCON_SRCS src/qrcon.cpp src/ServerManager.cpp)
set(BIN_QRCON_MOC_SRCS src/ServerManager.hpp)

# TODO: build this into the sourcedir (use generate_moc()) so that cpack
#       can generte it for the sourcebuild.
qt4_wrap_cpp(BIN_QRCON_MOC_OUTPUT ${BIN_QRCON_MOC_SRCS})
add_executable(${BIN_QRCON} ${BIN_QRCON_SRCS} ${BIN_QRCON_MOC_OUTPUT})

# TODO: nonportable - I want to provide the values in the toolchain file, but it
#       doesn't seem possible... how do you override computed values without 
#       going through ccmake?
#       NOTE: this should get solved when Ifinish patching findQt4 to take a
#       makespec
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows") 
  if (${CMAKE_CXX_COMPILER} MATCHES mingw) 
    target_link_libraries(${BIN_QRCON} -lQtCore4 -lQtGui4 ${LRCON_LIBRARIES})
    # TODO: should be for only one target, but there is no CPPFLAGS?  
    include_directories(/home/bunker/code/cross-compile/windows/include/Qt4
                        /home/bunker/code/cross-compile/windows/include/Qt4/Qt
                        /home/bunker/code/cross-compile/windows/include/Qt4/QtCore
                        /home/bunker/code/cross-compile/windows/include/Qt4/QtGui
                        )
  endif()
else()
  target_link_libraries(${BIN_QRCON} ${QT_LIBRARIES} ${LRCON_LIBRARIES})
endif()

######################
## Building Doxygen ##
######################

set(DOXYFILE_DEFAULT_FILE ${CMAKE_SOURCE_DIR}/Doxyfile.default)
set(DOXYFILE_OVERRIDES_FILE ${CMAKE_SOURCE_DIR}/Doxyfile.user-overrides)

set(
DOXYGEN_EXTRAS "EXAMPLE_PATH = ${CMAKE_SOURCE_DIR}/examples/ 
                INPUT = ${CMAKE_SOURCE_DIR}/include/ 
                PROJECT_NAME = ${PROJECT_NAME} 
                PROJECT_NUMBER = ${PROJECT_VERSION} 
                OUTPUT_DIRECTORY = ${CMAKE_BINARY_DIR}/doxygen/ 
                GENERATE_HTML = yes "
)

# TODO: add WANT_DOXYGEN_PDF to standard conf vars
# Note: could be expanded  to others like, WANT_DVI
if (WANT_DOXYGEN_PDF)
  set(DOXYGEN_EXTRAS "${DOXYGEN_EXTRAS} \nGENERATE_LATEX = yes \n")
endif()

# TODO: this is bruk if there is more than one doxygen targets!
# TODO: this should take a template file and not an overrides file.
# TODO: add the var for the templates file to the ccmake thingy; somehow
#       set a default that you can override if you want... no idea how
#       that works.
add_doxygen(
  ${DOXYFILE_DEFAULT_FILE} ${DOXYFILE_OVERRIDES_FILE} 
  ${DOXYGEN_EXTRAS}
)

# TODO: add WANT_DOXYGEN_REBUILD to vars and document it 
if (WANT_DOXYGEN_REBUILD)
  # TODO: add doxygen to the all target
endif()


##################
## Installation ##
##################

# Documentation
install(
  FILES README COPYING 
  DESTINATION doc/netz/
)

# TODO: The make doxygen target needs teh .md5 files (for rebuilds), but we want to 
#       get rid of them for installing.  How?  Custom command?
# TODO: If doc/ exists, then install it.  Otherwise, don't bother.  Then elsewhere
#       we should have an if REBUILD_DOXYGEN which causes the doxygen target to run
#       with make all.

if (WANT_DOXYGEN_PDF)
  #install pdf
endif()

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
# TODO: cpack should force moc to run (but that first requires that the mocfiles are 
#       build into the source which doesn't happen yet)