# qt building
include $(top_srcdir)/build-aux/autotroll.mk

bin_PROGRAMS = lrcon query
EXTRA_DIST = extra_todos.hpp master.hpp

lrcon_SOURCES = lrcon.cpp  
lrcon_CXXFLAGS = -Wall -pedantic
lrcon_CPPFLAGS = -I${top_srcdir}/include/ 
lrcon_CPPFLAGS += ${RCON_DEBUG_FLAGS}

query_SOURCES = query.cpp print_buf.hpp
query_CXXFLAGS = -Wall -pedantic
query_CPPFLAGS = -I${top_srcdir}/include/ 
query_CPPFLAGS += ${QUERY_DEBUG_FLAGS}

# Example of QT building:
#bin_PROGRAMS = lcdrange
#lcdrange_SOURCES = main.cc lcdrange.cpp lcdrange.moc.cpp lcdrange.h qt_main.cpp
#lcdrange_CXXFLAGS = $(QT_CXXFLAGS) $(AM_CXXFLAGS)
#lcdrange_CPPFLAGS = $(QT_CPPFLAGS) $(AM_CPPFLAGS)
#lcdrange_LDFLAGS  = $(QT_LDFLAGS) $(LDFLAGS)
#lcdrange_LDADD    = $(QT_LIBS) $(LDADD)
#BUILT_SOURCES = lcdrange.moc.cpp
