// Copyright (C) 2008 James Weber
// Under the LGPL3, see COPYING
/*!
\file
\brief Containing the entire \link common \endlink namespace.
*/

#ifndef COMMON_HPP_y3dykfiu
#define COMMON_HPP_y3dykfiu

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <endian.h>

#include <cstring>
#include <cassert>
#include <cerrno>

#include <stdexcept>
#include <iostream>

#if defined(RCON_DEBUG_MESSAGES) || defined(QUERY_DEBUG_MESSAGES)
#  define COMMON_DEBUG_MESSAGE(x__) std::cout << x__ << std::endl;
#else
#  define COMMON_DEBUG_MESSAGE(x__)
#endif

/*!
\def RCON_SYS_LITTLE_ENDIAN  

Readability variable for endianness feature test.  When it is defined, the byte 
order is swapped transparently.
*/

#ifndef __BYTE_ORDER
#  warning: assuming little endian byte order because __BYTE_ORDER does not exist.
#  define RCON_SYS_LITTLE_ENDIAN
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#  define RCON_SYS_LITTLE_ENDIAN
#elif __BYTE_ORDER == __BIG_ENDIAN
#  define RCON_SYS_BIG_ENDIAN
#else 
#  warning: assuming little endian byte order because __BYTE_ORDER is defined to something unknown.
#  define RCON_SYS_LITTLE_ENDIAN
#endif

//! \brief This is only used internally, but some docs are relevant.
//! 
//! Components which are not specific to a particular part of the lib.
namespace common {
  
  /*!
  \brief Take care of DNS and so on.  Prefer to use sub-classes.

  \todo This class is fine if you connect by DGRAM to a STREAM host, eg if 
        I connect to a webserver.  The error is not caught intil the recv
        call.  Any way I can trap it here?  Or elsewhere?
  
  \todo take an int for the port as well -- overloaded.  We can supply it as
        part of the hints.
  */
  class host {
    struct addrinfo *ad_info;
    
    public:
      //! Attributes for the constructor
      typedef enum {
        tcp   = 1 << 0, 
        udp   = 1 << 1, 
        is_ip = 1 << 2
      } host_attr_t;
      
      /*!
      \param attr  Bitmask of options from host_attr_t.  Defaults to tcp if nothing is set.
      */
      host(const char *host, const char *port, int attr = host::tcp) {
        COMMON_DEBUG_MESSAGE("Getting connection to: " << host << ":" << port << " a:" << attr);
        
        struct addrinfo hints;
        
        hints.ai_family = AF_UNSPEC;     /* Allow IPv4 (values: IPv6 AF_INET or AF_INET6) */
        assert(! (attr & tcp & udp));
        if (attr & udp) {
          hints.ai_socktype = SOCK_DGRAM;
        }
        else {
          hints.ai_socktype = SOCK_STREAM;
        }
        hints.ai_flags = AI_NUMERICSERV; /* Require a proper port number param */
        // optimisation when we know it's an IP already.
        if (attr & is_ip) hints.ai_flags |= AI_NUMERICHOST;
        hints.ai_protocol = 0;           /* Any protocol */
        hints.ai_canonname = NULL;
        hints.ai_addr = NULL;
        hints.ai_next = NULL;
  
        int r;
        if ((r = getaddrinfo(host, port, &hints, &ad_info)) != 0) {
          std::cerr << "gettaddrinfo(): " << gai_strerror(r) << std::endl;
          /// \todo better error
          throw std::runtime_error(gai_strerror(r));
        }
        
        if (ad_info->ai_addr == NULL || ad_info->ai_addrlen == 0) {
          /// \todo better error
          throw std::runtime_error("No socket address returned.");
          
        }
#ifdef RCON_DEBUG_MESSAGES
        else if (ad_info->ai_next != NULL) {
          COMMON_DEBUG_MESSAGE("Warning: more than one socket address was returned "
                               "from gettaddrinfo().  This could be a sign of protocol "
                               "changes or that the host is multi-homed.");
        }
#endif
      }
      
      ~host() {
        freeaddrinfo(ad_info);
      }
      
      //! ai_family for a socket() call.
      int family() const { return ad_info->ai_family; }
      
      //! Address struct for a connect() call.
      const struct sockaddr *address() const { return ad_info->ai_addr; }
      
      //! Length value for a connect() call.  
      int address_len() const { return ad_info->ai_addrlen; }
      
      //! SOCK_DGRAM etc.  For socket()
      int type() const { return ad_info->ai_socktype; }
  };
  
  //! \brief Non-instancable base class which resolves a circular dependancy from having authing.
  class connection_base {
    int socket_;
    
    protected:
      /*! 
      \brief Connects
      
      \bug connect() blocks forever if you connect to example.com:27015 -- can't I timeout?  
           Or find it in host?  This is when the host is dropping packets I guess.
      
      \todo Likely I need to use select() here to see if it's ready to be connected to.
      
      \todo resolve the problems with exceptions
      
      \throws connection_error  if socket() or connect() fails.
      */
      connection_base(const host &server) {
        COMMON_DEBUG_MESSAGE("Initialising sockets.");
        socket_ = ::socket(server.family(), server.type(), 0);
        if (socket_ == -1) {
          perror("socket()");
//           throw connection_error("socket() failed.");
          throw int(1);
        }
        
        if (connect(socket_, server.address(), server.address_len()) == -1) {
          perror("connect()");
//           throw connection_error("connect() failed.");
          throw int(1);
        }
        
        COMMON_DEBUG_MESSAGE("Got sockets.");
      }
      
      //! \brief Disconnects
      ~connection_base() {
        close(socket_);
      }
      
    public:
      //! Necessary to be public for the message types to call it but not the user.
      int socket() { return socket_; }
  };

  
  //! True if no timeout.  False otherwise.
  inline bool wait_for_select(int socket_fd) {
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(socket_fd, &rfds);
        
    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    if (select(socket_fd+1, &rfds, NULL, NULL, &timeout) == -1) {
      perror("select()");
      return false; // don't throw because the callers might need a different exception
    }
    
    // false if timed out
    return FD_ISSET(socket_fd, &rfds);
  }
  
  //! Copies from little endian to system endian.
  template <typename T>
  void endian_memcpy(T &destination, const void *source) {
#ifdef RCON_SYS_LITTLE_ENDIAN
    memcpy(&destination, source, sizeof(T));
#else
    char *dest = (char *) &destination;
    size_t j = 0;
    size_t i = sizeof(T)-1;
    while (i != -1) {
      dest[i] = source[j];
      --i; ++j;
    }
#endif
  }
  
}

#endif

