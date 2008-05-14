bin_PROGRAMS = lrcon query
EXTRA_DIST = extra_todos.hpp master.hpp

lrcon_SOURCES = lrcon.cpp  
lrcon_CXXFLAGS = -Wall -pedantic
lrcon_CPPFLAGS = -I${top_srcdir}/include/ 
lrcon_CPPFLAGS += -DRCON_DEBUG_MESSAGES

query_SOURCES = query.cpp print_buf.hpp
query_CXXFLAGS = -Wall -pedantic
query_CPPFLAGS = -I${top_srcdir}/include/
query_CPPFLAGS += -DQUERY_DEBUG_MESSAGES

