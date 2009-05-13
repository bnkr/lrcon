##############
## Prologue ##
##############
cmake_minimum_required(VERSION 2.6)
# For ease of use later
set(PROJECT_NAME lrcon)
set(PROJECT_VERSION "0.6.2")
project(${PROJECT_NAME})

include("build-aux/doxygen.cmake")
include("build-aux/btest.cmake")
include("build-aux/butil.cmake")

butil_standard_setup()

###########################
## Building normal stuff ##
###########################

# Global consts for libraries etc.
if(WIN32)
  set(LRCON_LIBRARIES -lws2_32)
else()
  set(LRCON_LIBRARIES "")
endif()

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

qt4_wrap_cpp(BIN_QRCON_MOC_OUTPUT ${BIN_QRCON_MOC_SRCS})
add_executable(${BIN_QRCON} ${BIN_QRCON_SRCS} ${BIN_QRCON_MOC_OUTPUT})
target_link_libraries(${BIN_QRCON} ${QT_LIBRARIES} ${LRCON_LIBRARIES})

######################
## Building Doxygen ##
######################

add_doxygen_directives(
  TARGET doxygen
  INPUTS "include/"
  ARGS_VAR ad_args
  DOCS_MIRROR "${CMAKE_SOURCE_DIR}/doc"
  DEFAULT_DOXYFILE "${CMAKE_SOURCE_DIR}/Doxyfile.default"
  INSTALL ${WANT_DOCS}
)
add_doxygen(${ad_args})

##################
## Installation ##
##################

# TODO: use butil auto installer
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

if (WIN32)
  message("Notice: windows binaries packages won't work without windows libs distributed.")
  set(WINDOWS_LIBS "build-aux/mingwm10.dll" "build-aux/QtCore4.dll" "build-aux/QtGui4.dll")
  message("Windows libs are ${WINDOWS_LIBS}")
  message("They can be symlinks, but this breaks everything except NSIS.")
  install(
    FILES ${WINDOWS_LIBS}
    DESTINATION ${BINDIR}
  )
endif()

#################
## CPack Stuff ##
#################

butil_cpack_setup(
  DESCRIPTION "RCON library and programs."
  LONG_DESCRIPTION
    "RCON protocol implementation, including programs for communicating with RCON servers."
  VENDOR      "bunker"
  EMAIL       "bunker@bunkerprivate.com"
  URL         "http://www.bunkerprivate.com/"
  BINARIES    "bin/qrcon" "bin/lrcon"
  RUNNABLES   "lrcon" "RCON command line"
              "qrcon" "RCON Qt GUI"
)

