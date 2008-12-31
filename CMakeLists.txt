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
option(WANT_DOXYGEN_PDF "Install PDF from doxygen (needs pdfLaTeX)" YES)

set(DOXYGEN_DOXYFILE_TEMPLATE "${CMAKE_SOURCE_DIR}/Doxyfile.default" CACHE STRING 
    "Base doxyfile.  Mostly useful for developers.  Certain vars will be overwritten by cmake (in/output paths mostly).")

mark_as_advanced(
  DOXYGEN_DOXYFILE_TEMPLATE 
  WANT_DOXYGEN_PDF_GRAPHICS 
  WANT_DOXYGEN_HTML_GRAPHICS 
  WANT_DOXYGEN_HTML WANT_DOXYGEN_PDF
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

# TODO: use list(APPEND output inputs...)

set(DOXYGEN_IN_SOURCEDIR "${CMAKE_SOURCE_DIR}/doc/")

list(APPEND DOXYGEN_FLAGS "EXAMPLE_PATH = ${CMAKE_SOURCE_DIR}/examples/")
list(APPEND DOXYGEN_FLAGS "INPUT = ${CMAKE_SOURCE_DIR}/include/")
list(APPEND DOXYGEN_FLAGS "PROJECT_NAME = ${PROJECT_NAME}")
list(APPEND DOXYGEN_FLAGS "PROJECT_NUMBER = ${PROJECT_VERSION}")
list(APPEND DOXYGEN_FLAGS "OUTPUT_DIRECTORY = ${CMAKE_BINARY_DIR}/doxygen/")

if (WANT_DOXYGEN_PDF)
  list(APPEND DOXYGEN_FLAGS "GENERATE_LATEX = yes")
endif()

if (WANT_DOXYGEN_HTML) 
  list(APPEND DOXYGEN_FLAGS "GENERATE_HTML = yes")
endif()

# TODO: horribly broken.  How do I return a variable by reference without 
#   using a macro?
set(DOXYGEN_TARGET "doxygen")

add_doxygen(
  DOXYGEN_INSTALLABLES
  ${DOXYGEN_TARGET}
  ${DOXYGEN_DOXYFILE_TEMPLATE}
  "${DOXYGEN_FLAGS}"
)

# TODO: i'm  pretty sure this is redundant now I have 
#   the file outputs done, but I still can't pass a value
#   back up the stack.
# if (WANT_DOXYGEN_REBUILD)
#   # add doxygen to all (add_dependancies usage isn't clear because `all' might not be
#   # the name of the ALL target)
#   add_custom_target(ALL DEPENDS ${DOXYGEN_TARGET})
# elseif(WANT_DOXYGEN_PDF OR WANT_DOXYGEN_HTML)
#   if (NOT EXISTS "${DOXYGEN_IN_SOURCEDIR}")
#     message(FATAL_ERROR "Documentation isn't available to install; set WANT_DOXYGEN_REBUILD or all the DOXYGEN targets to NO")
#   endif()
# endif()



##################
## Installation ##
##################

# autotools style
set(INCLUDEDIR "include/")
set(BINDIR "bin")
set(DATADIR "share/")
set(DOCDIR "${DATADIR}/doc/lrcon/")
set(HTMLDIR "${DATADIR}/doc/lrcon/")
set(PDFDIR "${DATADIR}/doc/lrcon/")
set(DVIDIR "${DATADIR}/doc/lrcon/")
set(PSDIR "${DATADIR}/doc/lrcon/")

message(STATUS "Adding install targets.")

if (WANT_DOCS) 
  # Documentation
  install(
    FILES README COPYING 
    DESTINATION ${DOCDIR}
  )
  
  if (WANT_DOXYGEN_HTML)
    install(
      DIRECTORY ${CMAKE_BINARY_DIR}/doxygen/html/
      DESTINATION ${DOCDIR}/html
      PATTERN "*.png"
      PATTERN "*.html"
      PATTERN "*.css"
    )
  endif()
  
  if (WANT_DOXYGEN_PDF)
    # TODO is docdir/lates the right place to install it?  It's nice for my release scripts... but not
    #   for a real release I think...
    install(
      FILES ${CMAKE_BINARY_DIR}/doxygen/latex/refman.pdf
      DESTINATION ${DOCDIR}/latex
    )
  endif()
  
endif(WANT_DOCS)

# Header files
install(FILES include/lrcon.hpp DESTINATION ${INCLUDEDIR})
install(
  FILES include/lrcon/query.hpp
        include/lrcon/rcon.hpp
        include/lrcon/common.hpp
  DESTINATION ${INCLUDEDIR}/lrcon/
)

# Binaries
install(TARGETS ${BIN_LRCON} DESTINATION ${BINDIR})
install(TARGETS ${BIN_QRCON} DESTINATION ${BINDIR})

#################
## CPack Stuff ##
#################

message(STATUS "Doing CPack stuff.")
message("Note: doc/ must be added into the source dir manually for a source package!")

# Note: Source package effectively snapshots the source directory, including copying
# symlinks.  This is how doxygen is built.  Therefore, to make a full release
# the current doxygen should be copied into a release dir instead of just symlinking
# to the developer built docs.  This way uploaded docs point to the actual docs for
# the release, not the next release!

# TODO: cpack should force doxygen to build: add_dependancies to package_source and package?
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
string(REGEX REPLACE "^[^.]+\\.[^.]+\\.[^\\-]-([0-9]+).*" "\\1" PROJECT_SNAPSHOT ${PROJECT_VERSION})
# TODO: is snapshot not handled by CPack?
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_PATCH})

set(CPACK_SOURCE_TGZ "ON")
set(CPACK_SOURCE_TBZ2 "OFF")
set(CPACK_SOURCE_TZ "OFF")

set(CPACK_BINARY_STGZ "OFF")
set(CPACK_BINARY_TBZ2 "OFF")
set(CPACK_BINARY_TGZ "ON")
set(CPACK_BINARY_TZ "OFF")

# Yes, you really need all that escape crap.
# TODO: make a user-defined var for extra stuff to ignore
set(CPACK_SOURCE_IGNORE_FILES "/\\\\..*/;~$;/releases/;/${CMAKE_BINARY_DIR}/;/source-server/;/_notes/;doc/html/.*\\\\.md5$;/_notes/;doc/html/.*\\\\.map$;Makefile")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}")


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

