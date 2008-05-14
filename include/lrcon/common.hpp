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

#include <cerrno>
#include <cstring>
#include <cassert>
#include <cerrno>
#include <cstring>

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
#  define LRCON_SYS_LITTLE_ENDIAN
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#  define LRCON_SYS_LITTLE_ENDIAN
#elif __BYTE_ORDER == __BIG_ENDIAN
#  define LRCON_SYS_BIG_ENDIAN
#else 
#  warning: assuming little endian byte order because __BYTE_ORDER is defined to something unknown.
#  define LRCON_SYS_LITTLE_ENDIAN
#endif

//! \brief This is only used internally, but some docs are relevant.
//! 
//! Components which are not specific to a particular part of the lib.
namespace common {
  //! When connect or getaddrinfo or socket fails...
  //! Catch-all error class.
  struct error : public std::runtime_error {
    error(const std::string &s) : std::runtime_error(s) {}
    ~error() throw() {}
  };
  
  //! Network error, indicating some kind of failure.
  struct network_error : public error {
    network_error(const std::string &s) : error(s) {}
    ~network_error() throw() {}
  };
  
  //! General communications errors which might be recoverable.
  struct comm_error : public error {
    comm_error(const std::string &s) : error(s) {}
    ~comm_error() throw() {}
  };
  
  
  //! A failure to connect(), socket() etc.
  struct connection_error : public comm_error {
    connection_error(const std::string &s) : comm_error(s) {}
    ~connection_error() throw() {}
  };
  
  //! A possibly recoverable error with authorisation.
  struct auth_error : public comm_error {
    auth_error(const std::string &s) : comm_error(s) {}
    ~auth_error() throw() {}
  };
  
  //! The password was wrong.
  struct bad_password : public comm_error {
    bad_password(const std::string &s) : comm_error(s) {}
    ~bad_password() throw() {}
  };
    
  //! Caused when the server responds with something unexpected (but not a true network or proto failure)
  struct response_error : public comm_error {
    response_error(const std::string &s) : comm_error(s) {}
    ~response_error() throw() {}
  };
  
  
  //! Sending data failed.
  struct send_error : public network_error {
    send_error(const std::string &s) : network_error(s) {}
    ~send_error() throw() {}
  };
  
  //! Reading data failed.
  struct recv_error : public network_error {
    recv_error(const std::string &s) : network_error(s) {}
    ~recv_error() throw() {}
  };
  
  //! Caused by some violation of the protocol.
  struct proto_error : public network_error {
    proto_error(const std::string &s) : network_error(s) {}
    ~proto_error() throw() {}
  };
  
  /*!
  \brief Take care of DNS and so on.  Prefer to use sub-classes.

  \todo This class is fine if you connect by DGRAM to a STREAM host, eg if 
        I connect to a webserver.  The error is not caught intil the recv
        call.  Any way I can trap it here?  Or elsewhere?
  
  \todo take an int for the port as well -- overloaded.  We can supply it as
        part of the hints.  Then you can just htonl it and put it in the hints 
        I think.
  
  \todo Better way of determining is_ip.  Bitmasks suck.  Better host(ip("127....));
        and host(string("sdfsdf").  Bit iffy tho.  How does the user code make that
        nice?

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
        COMMON_DEBUG_MESSAGE("Host is: " << host << ":" << port << " a:" << attr);
        
        struct addrinfo hints;
        
        hints.ai_family = AF_UNSPEC;     /* values: IPv6 AF_INET or AF_INET6 */
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
          throw connection_error(std::string("gettaddrinfo() failed: ") + gai_strerror(r));
        }
        
        if (ad_info->ai_addr == NULL || ad_info->ai_addrlen == 0) {
          throw connection_error("No socket address returned.");
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
      */
      connection_base(const host &server) {
        COMMON_DEBUG_MESSAGE("Initialising sockets.");
        socket_ = ::socket(server.family(), server.type(), 0);
        if (socket_ == -1) {
          throw connection_error(std::string("socket() failed: ") + strerror(errno));
        }
        
        if (connect(socket_, server.address(), server.address_len()) == -1) {
          throw connection_error(std::string("connect() failed: ") + strerror(errno));
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

  
  //! True if no timeout.  False otherwise.  Time values are offsets.
  inline bool wait_for_select(int socket_fd, int seconds = 1, int usecs = 0) {
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(socket_fd, &rfds);
        
    struct timeval timeout;
    timeout.tv_sec = seconds;
    timeout.tv_usec = usecs;
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
#ifdef LRCON_SYS_LITTLE_ENDIAN
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
  
#ifdef LRCON_SYS_LITTLE_ENDIAN
  template <typename T>
  static T rcon_to_native_endian(T x) {
    return x;
  }
  
  template <typename T>
  static T native_to_rcon_endian(T x) {
    return x;
  }
#else
  template <typename T>
  static T swap_endian(T x) {
    size_t i = 0, j = sizeof(T) - 1;
    char t;
    char *y = (char *) &x;
    size_t i;
    while (i < j) {
      t = y[i];
      y[i] = y[j];
      y[j] = t;
      --j; ++i;
    }
    return x;
  }
  
  template <typename T>
  static T server_to_native_endian(T x) {
    return swap_endian(x);
  }
  
  template <typename T>
  static T native_to_server_endian(T x) {
    return swap_endian(x);
  }
#endif
  
  /*! 
  \brief Reads an integral type, ensureing endianness.
  \returns bytes sent or -1 on error
  */
  template<typename T>
  bool read_type(int socket, T &v) {
    int bytes = recv(socket, &v, sizeof(T), 0);
    v = server_to_native_endian(v);
    return bytes;
  }
  
  /*! 
  \brief Convenience wrapper, ensuring endianness.
  \returns bytes sent or -1 on error
  */
  template <typename T>
  bool send_type(int socket, T v) {
    v = native_to_server_endian(v);
    int bytes = send(socket, &v, sizeof(T), 0);
    return bytes;
  }
}

#endif

