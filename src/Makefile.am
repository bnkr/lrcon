# qt building
include $(top_srcdir)/build-aux/autotroll.mk

ACLOCAL_AMFLAGS = -I build-aux

EXTRA_DIST = extra_todos.hpp master.hpp

bin_PROGRAMS = lrcon query

# for messing around.. need more missions to make this work properly.
LRCON_WIN_LIBS = -lws2_32

lrcon_SOURCES = lrcon.cpp  
lrcon_CXXFLAGS = -Wall -pedantic ${AM_CXXFLAGS}
lrcon_CPPFLAGS = -I${top_srcdir}/include/ ${AM_CPPFLAGS}
lrcon_CPPFLAGS += ${RCON_DEBUG_FLAGS}
lrcon_LDADD = ${LRCON_WIN_LIBS}

query_SOURCES = query.cpp print_buf.hpp
query_CXXFLAGS = -Wall -pedantic ${AM_CXXFLAGS}
query_CPPFLAGS = -I${top_srcdir}/include/  ${AM_CPPFLAGS}
query_CPPFLAGS += ${QUERY_DEBUG_FLAGS}
query_LDADD = ${LRCON_WIN_LIBS}

BUILT_SOURCES = ServerManager.moc.cpp

bin_PROGRAMS += qrcon
qrcon_SOURCES  = ServerManager.hpp ServerManager.cpp ServerManager.moc.cpp qrcon.cpp
qrcon_CXXFLAGS = $(QT_CXXFLAGS) $(AM_CXXFLAGS)  
qrcon_CPPFLAGS = $(QT_CPPFLAGS) $(AM_CPPFLAGS) -I ${top_srcdir}/include/
qrcon_LDFLAGS  = $(QT_LDFLAGS) $(LDFLAGS)
qrcon_LDADD    = $(QT_LIBS) $(LDADD) ${LRCON_WIN_LIBS}

