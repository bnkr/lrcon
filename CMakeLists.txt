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

# General options
option(WANT_DOXYGEN_HTML "Install (generate if necessary) HTML made by doxygen" YES)
option(WANT_DOXYGEN_PDF "Install (generate if necessary) PDF from doxygen" NO)
option(WANT_DOXYGEN_REBUILD "Rebuild doxygen stuff instead of using what's in the tarball (needs doxygen)" NO)

option(WANT_DOXYGEN_PDF_GRAPHICS "Use the graphical stuff in pdfs (needs dot, LaTeX graphics converters)" YES)
option(WANT_DOXYGEN_HTML_GRAPHICS "Use the graphical stuff in htmls (needs dot)" YES)

# Set it to itself so that this var can be in the toolchain file
set(QT_QMAKE_SPEC_FILE "${QT_QMAKE_SPEC_FILE}" CACHE STRING 
    "Manually defined qmake spec files.  Used by patched Qt4 finder to ease cross-compiling.")

###########################
## Building normal stuff ##
##########################

# Lrcon binary stuff
set(BIN_LRCON "lrcon")
add_executable(${BIN_LRCON} src/lrcon.cpp )
target_link_libraries(${BIN_LRCON} ${LRCON_LIBRARIES})

####################
## Building Qrcon ##
####################

find_package(Qt4)
include(${QT_USE_FILE})

# # TODO: ok this is a real pain.  It seems to be almost impossible to get qmake to report the 
# #   correct values.  It looks like the only way is to define all this crap in one go using ccmake
# #   and cmake.  
# #   - The stupid thing refuses to find qmake, despite it also finding moc,uic etc.
# #   - Won't find *any* of the libs or headers 
# #     - that's expected since I don't have the win headers here, but why is there
# #       no way to add the location of windows in?!  
# #   SO: can I use the toolchain file to define all this bullshit statically, or is it going to
# #   make me use a script and/or ccmake every time?
# if(QT_USE_CMAKE_FINDER)
#   find_package(Qt4)
#   include(${QT_USE_FILE})
# else()
#   include("build-aux/FindQt4.cmake")
#   include(${QT_USE_FILE})
# endif()

set(BIN_QRCON "qrcon")
set(BIN_QRCON_SRCS src/qrcon.cpp src/ServerManager.cpp)
set(BIN_QRCON_MOC_SRCS src/ServerManager.hpp)

# TODO: build this into the sourcedir (use generate_moc()) so that cpack
#       can generte it for the sourcebuild.
qt4_wrap_cpp(BIN_QRCON_MOC_OUTPUT ${BIN_QRCON_MOC_SRCS})
add_executable(${BIN_QRCON} ${BIN_QRCON_SRCS} ${BIN_QRCON_MOC_OUTPUT})
target_link_libraries(${BIN_QRCON} ${QT_LIBRARIES} ${LRCON_LIBRARIES})

######################
## Building Doxygen ##
######################

set(DOXYGEN_DOXYFILE_TEMPLATE "${CMAKE_SOURCE_DIR}/Doxyfile.default" CACHE STRING 
    "Base doxyfile.  Certain vars will be overwritten (in/output paths mostly).")

# TODO: Generate this string with a lib function; pretend it's a struct.
set(
DOXYGEN_EXTRAS "EXAMPLE_PATH = ${CMAKE_SOURCE_DIR}/examples/ 
                INPUT = ${CMAKE_SOURCE_DIR}/include/ 
                PROJECT_NAME = ${PROJECT_NAME} 
                PROJECT_NUMBER = ${PROJECT_VERSION} 
                OUTPUT_DIRECTORY = ${CMAKE_BINARY_DIR}/doxygen/ "
)

if (WANT_DOXYGEN_PDF)
  set(DOXYGEN_EXTRAS "${DOXYGEN_EXTRAS} \nGENERATE_LATEX = yes \n")
endif()

if (WANT_DOXYGEN_HTML) 
  set(DOXYGEN_EXTRAS "${DOXYGEN_EXTRAS} \nGENERATE_HTML = yes \n")
endif()

add_doxygen(
  doxygen
  ${DOXYGEN_DOXYFILE_TEMPLATE}
  ${DOXYGEN_EXTRAS}
)

if (WANT_DOXYGEN_REBUILD)
  # TODO: add doxygen to the all target
else()
  # TODO: check the doc target exists or error
endif()

##################
## Installation ##
##################

# Documentation
# TODO: this goes to the wrong place: usr/local/doc/netz; should be /usr/local/share/doc 
install(
  FILES README COPYING 
  DESTINATION doc/netz/
)

# TODO: The make doxygen target needs teh .md5 files (for rebuilds), but we want to 
#       get rid of them for installing.  How?  Custom command?

if (WANT_DOXYGEN_HTML) 
  # if rebuild, install from binary dir else source dir
endif()

if (WANT_DOXYGEN_PDF)
  # if rebuild, install from binary dir else source dir
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