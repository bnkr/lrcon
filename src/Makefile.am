bin_PROGRAMS = lrcon query
EXTRA_DIST = extra_todos.hpp master.hpp

include_HEADERS = include/lrcon.hpp
include_lrcon_HEADERS = include/lrcon/rcon.hpp include/lrcon/common.hpp include/lrcon/query.hpp
include_lrcondir = ${includedir}/lrcon/

lrcon_SOURCES = lrcon.cpp  
lrcon_CXXFLAGS = -Wall -pedantic
lrcon_CPPFLAGS = -I${srcdir}/include/
#lrcon_CPPFLAGS += -DRCON_DEBUG_MESSAGES

query_SOURCES = query.cpp print_buf.hpp
query_CXXFLAGS = -Wall -pedantic
query_CPPFLAGS = -I${srcdir}/include/
query_CPPFLAGS += -DQUERY_DEBUG_MESSAGES

