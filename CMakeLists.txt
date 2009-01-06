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
option(WANT_DOXYGEN_REBUILD "Regenerate doxygen and install that instead of the distributed docs." NO)

set(DOXYGEN_DOXYFILE_TEMPLATE "${CMAKE_SOURCE_DIR}/Doxyfile.default" CACHE STRING 
    "Base doxyfile.  Mostly useful for developers.  Certain vars will be overwritten by cmake (in/output paths mostly).")

mark_as_advanced(
  DOXYGEN_DOXYFILE_TEMPLATE 
  WANT_DOXYGEN_REBUILD
  WANT_DOXYGEN_HTML WANT_DOXYGEN_PDF
)

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

if (WANT_DOXYGEN_PDF)
  list(APPEND DOXYGEN_WANTS "pdf")
endif()
if (WANT_DOXYGEN_HTML)
  list(APPEND DOXYGEN_WANTS "html")
endif()

set(DOXYGEN_CMAKE_VERBOSE "yes")

set(DOXYGEN_TARGET "doxygen")
doxgyen_setup_flags(
  DOXYGEN_FLAGS 
  "${DOXYGEN_TARGET}"
  "${DOXYGEN_WANTS}"
)

add_doxygen(
  ${DOXYGEN_TARGET}
  ${DOXYGEN_DOXYFILE_TEMPLATE}
  "${DOXYGEN_FLAGS}"
)

##################
## Installation ##
##################

message(STATUS "Adding install targets.")

set(INCLUDEDIR "include/")
set(BINDIR "bin")
set(DATADIR "share/")
set(LIBDIR "lib/")
set(DOCDIR "${DATADIR}/doc/lrcon/")
set(HTMLDIR "${DATADIR}/doc/lrcon/")
set(PDFDIR "${DATADIR}/doc/lrcon/")
set(DVIDIR "${DATADIR}/doc/lrcon/")
set(PSDIR "${DATADIR}/doc/lrcon/")


if (WANT_DOCS) 
  # Documentation
  install(
    FILES README COPYING 
    DESTINATION ${DOCDIR}
  )

  if (WANT_DOXYGEN_REBUILD)
    set(INSTALL_FROM "")
    message("Selected rebuilt doxygen.")
  else()
    set(INSTALL_FROM "${CMAKE_SOURCE_DIR}/doc/")
    message("Installing distributed doxygen.")
  endif()

  doxygen_install_targets(
    "${DOXYGEN_TARGET}" 
    "${DOXYGEN_WANTS}" 
    "${DOCDIR}" 
    "${INSTALL_FROM}"
  )
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
install(
  TARGETS ${BIN_LRCON} ${BIN_QRCON}
  RUNTIME DESTINATION ${BINDIR}
)


#################
## CPack Stuff ##
#################

message(STATUS "Doing CPack stuff.")
message("Note: latest doc/ must be added into the source dir manually for a source package!")

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "RCON protocol implementation, including programs for communicating with RCON servers.")
set(CPACK_PACKAGE_VENDOR "bunkersoft")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/build-aux/install-readme.txt")

string(REGEX REPLACE "^([0-9]+).*" "\\1" PROJECT_MAJOR ${PROJECT_VERSION})
string(REGEX REPLACE "^[^.]+\\.([0-9]+).*" "\\1" PROJECT_MINOR ${PROJECT_VERSION})
string(REGEX REPLACE "^[^.]+\\.[^.]+\\.([0-9]+).*" "\\1" PROJECT_PATCH ${PROJECT_VERSION})
string(REGEX REPLACE "^[^.]+\\.[^.]+\\.[^\\-]-([0-9]+).*" "\\1" PROJECT_SNAPSHOT ${PROJECT_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_PATCH})

set(CPACK_SOURCE_TGZ "ON")
set(CPACK_SOURCE_TBZ2 "OFF")
set(CPACK_SOURCE_TZ "OFF")
set(CPACK_SOURCE_ZIP "OFF")

set(CPACK_BINARY_STGZ "OFF")
set(CPACK_BINARY_TBZ2 "OFF")
set(CPACK_BINARY_TGZ "OFF")
set(CPACK_BINARY_TZ "OFF")

# Yes, you really need all that escape crap.
set(CPACK_SOURCE_IGNORE_FILES "/\\\\..*/;~$;.*\\\\.swp$;/releases/;/${CMAKE_BINARY_DIR}/;/;/_notes/;doc/html/.*\\\\.md5$;/_notes/;doc/html/.*\\\\.map$;Makefile")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}")

set(CPACK_PACKAGE_EXECUTABLES "lrcon" "RCON command line" "qrcon" "RCON Qt")

set(CPACK_PACKAGE_INSTALL_DIRECTORY "lrcon ${PROJECT_VERSION}")
if(WIN32)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  # set(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
  # set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\MyExecutable.exe")
  set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} Lrcon programs and library.")
  set(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.bunkerprivate.com")
  set(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.bunkerprivate.com")
  set(CPACK_NSIS_CONTACT "bunkerprivate@googlemail.com")
  set(CPACK_NSIS_MODIFY_PATH ON)
else()
  set(CPACK_STRIP_FILES "bin/lrcon" "bin/qrcon")
  set(CPACK_SOURCE_STRIP_FILES "")
endif()


include(CPack)

