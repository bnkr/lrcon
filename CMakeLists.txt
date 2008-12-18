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
#       how can I put a default in there?  Or do I overwrite
#       ccmake?  It's not clear at all!
# set(CMAKE_C_FLAGS_DEBUG "-g -Wall")
# set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall")
# set(CMAKE_C_FLAGS_RELEASE "-O3 -Wall")
# set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall")

# Global consts for libraries etc.
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(LRCON_LIBRARIES -lws2_32)
else()
  set(LRCON_LIBRARIES "")
endif()

# General options
option(WANT_DOCS "Install documentation - when off, all other docs options are off." YES)
option(WANT_DOXYGEN_HTML "Install HTML made by doxygen" YES)
option(WANT_DOXYGEN_PDF "Install PDF from doxygen (needs pdfLaTeX)" NO)
option(WANT_DOXYGEN_REBUILD "Rebuild doxygen stuff instead of using what's in the tarball (needs doxygen)" NO)

option(WANT_DOXYGEN_PDF_GRAPHICS "Use the graphical stuff in pdfs (needs dot, LaTeX graphics converters)" YES)
option(WANT_DOXYGEN_HTML_GRAPHICS "Use the graphical stuff in htmls (needs dot)" YES)

set(DOXYGEN_DOXYFILE_TEMPLATE "${CMAKE_SOURCE_DIR}/Doxyfile.default" CACHE STRING 
    "Base doxyfile.  Mostly useful for developers.  Certain vars will be overwritten (in/output paths mostly).")

mark_as_advanced(
  DOXYGEN_DOXYFILE_TEMPLATE 
  WANT_DOXYGEN_PDF_GRAPHICS 
  WANT_DOXYGEN_HTML_GRAPHICS 
  WANT_DOXYGEN_HTML 
  WANT_DOXYGEN_PDF
)

# Set it to itself so that this var can be in the toolchain file
# set(QT_QMAKE_SPEC_FILE "${QT_QMAKE_SPEC_FILE}" CACHE STRING 
#     "Manually defined qmake spec files.  Used by patched Qt4 finder to ease cross-compiling.")

###########################
## Building normal stuff ##
###########################

# Lrcon binary stuff
set(BIN_LRCON "lrcon")
add_executable(${BIN_LRCON} src/lrcon.cpp )
target_link_libraries(${BIN_LRCON} ${LRCON_LIBRARIES})

####################
## Building Qrcon ##
####################

find_package(Qt4)
include(${QT_USE_FILE})

# TODO: remove this when I've worked out how to define it per-compile (or better, to default
#       the debug flags)
#add_definitions("-DRCON_DEBUG_MESSAGES -DQRCON_DEBUG_MESSAGES")

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

# Truly horrible list append code here :)

set(DOXYGEN_FLAGS "EXAMPLE_PATH = ${CMAKE_SOURCE_DIR}/examples/")
set(DOXYGEN_FLAGS "${DOXYGEN_FLAGS}\;INPUT = ${CMAKE_SOURCE_DIR}/include/")
set(DOXYGEN_FLAGS "${DOXYGEN_FLAGS}\;PROJECT_NAME = ${PROJECT_NAME}")
set(DOXYGEN_FLAGS "${DOXYGEN_FLAGS}\;PROJECT_NUMBER = ${PROJECT_VERSION}")
# TODO: for a maintainer build, doxygen should do into the source tree since
#       I need to distribute it as part of the tarball.  Perhaps I can solve
#       some of this with CPack?  Maybe CPack can manually add in built 
#       doxygen to the source tarball... or just follow a symlink?
set(DOXYGEN_FLAGS "${DOXYGEN_FLAGS}\;OUTPUT_DIRECTORY = ${CMAKE_BINARY_DIR}/doxygen/")

if (WANT_DOXYGEN_PDF)
  set(DOXYGEN_FLAGS "${DOXYGEN_FLAGS}\;GENERATE_LATEX = yes")
endif()

if (WANT_DOXYGEN_HTML) 
  set(DOXYGEN_FLAGS "${DOXYGEN_FLAGS}\;GENERATE_HTML = yes")
endif()

add_doxygen(
  doxygen
  ${DOXYGEN_DOXYFILE_TEMPLATE}
  ${DOXYGEN_FLAGS}
)

if (WANT_DOXYGEN_REBUILD)
  # TODO: add doxygen to the all target
elseif(WANT_DOXYGEN_PDF OR WANT_DOXYGEN_HTML)
  if (NOT EXISTS "${CMAKE_SOURCE_DIR}/doc/")
    message(FATAL_ERROR "Documentation doesn't exist yet; set WANT_DOXYGEN_REBUILD or unset WANT_DOCS")
  endif()
endif()

##################
## Installation ##
##################

# Documentation
# TODO: this goes to the wrong place: usr/local/doc/lrcon; should be /usr/local/share/doc/lrcon/

install(
  FILES README COPYING 
  DESTINATION doc/lrcon/
)
  
# TODO: The make doxygen target needs teh .md5 files (for rebuilds), but we want to 
#       get rid of them for installing.  How?  Custom command?
  
if (WANT_DOXYGEN_HTML) 
  if (WANT_DOXYGEN_REBUILD) 
    # install from binarydir
  else()
    # install from source dir
  endif()
endif()

if (WANT_DOXYGEN_PDF)
  if (WANT_DOXYGEN_REBUILD) 
    # install from binarydir
  else()
    # install from source dir
  endif()
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

# Note: Source package effectively snapshots the source directory, including copying
# symlinks.  This is how doxygen is built.  Therefore, to make a full release
# the current doxygen should be copied into a release dir instead of just symlinking
# to the developer built docs.  This way uploaded docs point to the actual docs for
# the release, not the next release!

# TODO: cpack should force doxygen to build: add_dependancies to package_source and package?
# TODO: we should only allow the release doxyfile, not a user-defined one
# TODO: cpack should force moc to run (but that first requires that the mocfiles are 
#       build into the source which doesn't happen yet)

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "My funky project")
set(CPACK_PACKAGE_VENDOR "Me, myself, and I")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/build-aux/install-readme.txt")

string(REGEX REPLACE "^([0-9]+).*" "\\1" PROJECT_MAJOR ${PROJECT_VERSION})
string(REGEX REPLACE "^[^.]+\\.([0-9]+).*" "\\1" PROJECT_MINOR ${PROJECT_VERSION})
string(REGEX REPLACE "^[^.]+\\.[^.]+\\.([0-9]+).*" "\\1" PROJECT_PATCH ${PROJECT_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_PATCH})

set(CPACK_SOURCE_TGZ "ON")
set(CPACK_SOURCE_TBZ2 "OFF")
set(CPACK_SOURCE_TZ "OFF")

set(CPACK_BINARY_STGZ "OFF")
set(CPACK_BINARY_TBZ2 "OFF")
set(CPACK_BINARY_TGZ "OFF")
set(CPACK_BINARY_TZ "OFF")

# Yes, you really need all that escape crap.
# TODO: make a user-defined var for extra stuff to ignore
set(CPACK_SOURCE_IGNORE_FILES "/\\\\..*/;~$;/releases/;/cmake-build-dir/;/source-server/;/_notes/;doc/html/.*\\\\.md5$;/_notes/;doc/html/.*\\\\.map$;Makefile")
set(CPACK_PACKAGE_EXECUTABLES "MyExecutable" "My Executable")

set(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
if(WIN32 AND NOT UNIX)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  # TODO: all of this, but it requires cross-compile anyway
  set(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
  set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\MyExecutable.exe")
  set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} My Famous Project")
  set(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.my-project-home-page.org")
  set(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.my-personal-home-page.com")
  set(CPACK_NSIS_CONTACT "me@my-personal-home-page.com")
  set(CPACK_NSIS_MODIFY_PATH ON)
else()
  set(CPACK_STRIP_FILES "bin/MyExecutable")
  
  set(CPACK_SOURCE_STRIP_FILES "")
endif()


include(CPack)

