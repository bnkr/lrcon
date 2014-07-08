##############
## Prologue ##
##############
cmake_minimum_required(VERSION 2.6)
# For ease of use later
set(PROJECT_NAME lrcon)
set(PROJECT_VERSION "0.6.2")
project(${PROJECT_NAME})

include_directories("include")

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

##################
## Installation ##
##################

# TODO: no such thing yet
