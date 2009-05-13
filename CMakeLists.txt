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
butil_auto_install(
  RUNNABLES_VAR runnables
  RUNNABLES "${BIN_LRCON}" "RCON command line"
            "${BIN_QRCON}" "RCON Qt GUI"
  BINARIES_VAR stuff_to_strip
  TARGETS ${BIN_LRCON} ${BIN_QRCON}
  # INCLUDE_DIRS dir...
  # [HEADER_EXCLUDE pattern... | NO_HEADERS]
  # WINDOWS_LIBS library...]
  # Use ln to put the dlls into bindir instead of copy.
  HARDLINK_AUX_DLLS
  # Always search for dlls to install in build-aux.
  BUILD_AUX_DLLS
)

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
  BINARIES    ${stuff_to_strip}
  RUNNABLES   ${runnables}
)

