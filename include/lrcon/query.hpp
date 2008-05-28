// Copyright (C) 2008 James Weber
// Under the LGPL3, see COPYING
/*!
\file
\brief The public query interface to a server.

\internal

\todo Main Todos:
      - work out how the sequencing/out-of-order packets thing will work (hard because
        I need a way to test it)
      - implement full data structures for the returned data.
      - implement queries for stuff other than counter-strike source.



*/

#ifndef QUERY_HPP_ifzth5xs
#define QUERY_HPP_ifzth5xs

#include <lrcon/common.hpp>

#ifdef QUERY_DEBUG_MESSAGES
#  include <iostream>
#  define QUERY_DEBUG_MESSAGE(x__) std::cout << x__ << std::endl; 
#else
#  define QUERY_DEBUG_MESSAGE(x__)
#endif

//! For querying the public properties of servers.
namespace query {
  //! Convenience wrapper class
  struct host : public common::host {
    //! \param is_ip  means no lookup will be done if true
    host(const char *host, const char *port, bool is_ip = false) 
    : common::host(host, port, ((is_ip) ? (common::host::is_ip&common::host::udp) : common::host::udp)) {}
  };
  
  //! Wrapper for a query server connection.
  class connection : public common::connection_base {
    public:
      //! Connect to the given query server.
      connection(const host &server) : common::connection_base(server) {}
      
    protected:
      //! override the access.
      int socket() { return connection_base::socket(); }
  };
  


  //! Non-instanciable base class for request types.
  class query_base {
    protected:
      static const size_t max_packet_size = 1400;
      enum split_type {split_single = -1, split_multiple = -2};
  };
  
  
#if 0


  //! List of players.
  class players : public query_base {
    public:
      players(connection &conn) : query_base() {}
  };
  
  //! Configuration variables.
  class rules : public query_base {
    public:
      rules(connection &conn) : query_base() {}
  };
  
#endif
  

}




#endif


