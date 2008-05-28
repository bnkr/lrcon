#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([lrcon], [0.5])
# extra stuff for building
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall -Werror])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_LANG(C++)

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([cassert cerrno cstdio cstdlib cstring ctime endian.h iostream stdexcept string])
AC_CHECK_HEADERS([arpa/inet.h netinet/in.h sys/socket.h unistd.h netdb.h sys/time.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_INT32_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_TYPE_INT16_T
AC_TYPE_INT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_SELECT_ARGTYPES
AC_CHECK_FUNCS([select socket getaddrinfo])
AC_CHECK_FUNCS([strerror])

# Options

enableval="no"
AC_ARG_ENABLE([debug-rcon], 
              AS_HELP_STRING([--enable-debug-rcon], [Turn on debugging output for lrcon binary.]) )

RCON_DEBUG_FLAGS=""
if test x$enableval != xno ; then
  echo "info: debug for rcon is turned on."
  RCON_DEBUG_FLAGS="-DRCON_DEBUG_MESSAGES"
fi
AC_SUBST([RCON_DEBUG_FLAGS])

enableval="no"
AC_ARG_ENABLE([debug-query], 
              AS_HELP_STRING([--enable-debug-query], [Turn on debugging output for query binary.]) )

QUERY_DEBUG_FLAGS=""
if test x$enableval != xno ; then
  echo "info: debug for query is turned on."
  QUERY_DEBUG_FLAGS="-DQUERY_DEBUG_MESSAGES"
fi
AC_SUBST([QUERY_DEBUG_FLAGS])

## magic to make qt work (autotroll package)
AT_WITH_QT

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
  Makefile
  src/Makefile
  include/Makefile
])

AC_OUTPUT
