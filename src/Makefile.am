# qt building
include $(top_srcdir)/build-aux/autotroll.mk

ACLOCAL_AMFLAGS = -I build-aux

EXTRA_DIST = extra_todos.hpp master.hpp 

bin_PROGRAMS = lrcon 
lrcon_SOURCES = lrcon.cpp  
lrcon_CXXFLAGS = -Wall -pedantic ${AM_CXXFLAGS}
lrcon_CPPFLAGS = -I${top_srcdir}/include/ ${AM_CPPFLAGS}
lrcon_CPPFLAGS += ${RCON_DEBUG_FLAGS}
lrcon_LDADD = ${LRCON_LDADD}

#bin_PROGRAMS += query
#query_SOURCES = query.cpp print_buf.hpp
#query_CXXFLAGS = -Wall -pedantic ${AM_CXXFLAGS}
#query_CPPFLAGS = -I${top_srcdir}/include/  ${AM_CPPFLAGS}
#query_CPPFLAGS += ${QUERY_DEBUG_FLAGS}
#query_LDADD = ${LRCON_LDADD}

BUILT_SOURCES = ServerManager.moc.cpp

bin_PROGRAMS += qrcon
qrcon_SOURCES  = common.hpp ServerManager.hpp ServerManager.cpp ServerManager.moc.cpp qrcon.cpp
qrcon_CXXFLAGS = $(QT_CXXFLAGS) $(AM_CXXFLAGS)  
qrcon_CPPFLAGS = $(QT_CPPFLAGS) $(AM_CPPFLAGS) -I ${top_srcdir}/include/
qrcon_LDFLAGS  = $(QT_LDFLAGS) $(LDFLAGS)
qrcon_LDADD    = $(QT_LIBS) $(LDADD) ${LRCON_LDADD}
# Windows only works this way.  Autotroll doesn't work out the defines properly...
#qrcon_LDADD    = -lQtCore4 -lQtGui4 $(LDADD) ${LRCON_LDADD}
