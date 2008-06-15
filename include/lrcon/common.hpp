// Copyright (C) 2008 James Weber
// Under the LGPL3, see COPYING
/*!
\file
\brief Containing the entire \link common \endlink namespace.
*/

/// \todo Is it nicer to have wait_for_select, read and co. in the connection
///       datatype.  Then we just always work with this connection?  Better for
///       a lib maybe, of course, the read<T> type functions are nice for reading
///       from an arbitrary buffer anyway.  If it happens the other way then it
///       could be made ostream like.  Lots more stuff needs to be parameterised
///       for the lib anyway so the current design is prolly ok.  Also, these
///       atributes like type() should be hidden; host is more like an opaque
///       type.

#ifndef COMMON_HPP_y3dykfiu
#define COMMON_HPP_y3dykfiu

// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netdb.h>

// #include <config.h>

/// stuffs on win vs. unix: http://tangentsoft.net/wskfaq/articles/bsd-compatibility.html

/// \todo WSAGetLastError() is needed to get errno; also the actual numbers are different (but gai_strerror would work 
///       (not threadsafe aparently))

/// \todo This whole portability segment is messed.

#ifdef HAVE_WINSOCK_H
#define LRCON_WINDOWS
#endif 

#ifdef LRCON_WINDOWS
// there is no getaddrinfo etc. until winxp [later there might be portable wrappers in mingw]
// #  define WINVER WindowsXP /* <-- doesn't work ?! */
#  define _WIN32_WINNT 0x501 /* this isn't the right way to do it! */
#  include <winsock2.h>
#  include <ws2tcpip.h>
#else
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <netdb.h>
#endif

#ifdef HAVE_ENDIAN_H
#include <endian.h>
#endif

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

/*! 
\brief This is only used internally, but some docs are relevant.

Components which are not specific to a particular part of the lib.
*/
namespace common {
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
  
  //! An unexpected timeout
  struct timeout_error : public comm_error {
    timeout_error(const std::string &s) : comm_error(s) {}
    ~timeout_error() throw() {}
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
  
  
  //! Throw given exception using errno to get a message
  template <typename Exception>
  void errno_throw(const char *message) {
    throw Exception(std::string(message) + ": " + strerror(errno));
  }
  
#ifdef LRCON_WINDOWS
  //! Not threadsafe and generally a big hack.  Perhaps it should be made that 
  //! you just link to a static lib which does this statically.
  inline int winsock_init() {
    static WSADATA w;
    static bool done = false;
    if (! done) {
      int error = WSAStartup(0x0202, &w);
  
      if (error) {
        /// better exception needed
        throw connection_error("Winsock error; couldn't run winsock startup.");
        // do something?!
      }
      if (w.wVersion != 0x0202) { // wrong WinSock version!
        /// does this need to be called always?  It's annoying.  Really there needs an
        /// object somewhere which does this like class network;.  Shame that the bsd
        /// implementers would need to use it also.
        WSACleanup(); // unload ws2_32.dll
        throw connection_error("Wrong winsock version.");
      }
      done = true;
      return error;
    }
    return 0;
  }
#endif
  
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
#ifdef LRCON_WINDOWS
        winsock_init();
#endif
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
        hints.ai_flags = 0;
        // win doesn't appear to have this
#ifndef LRCON_WINDOWS
        hints.ai_flags = AI_NUMERICSERV; /* Require a proper port number param */
#endif
        // optimisation when we know it's an IP already.
        if (attr & is_ip) hints.ai_flags |= AI_NUMERICHOST;
        hints.ai_protocol = 0;           /* Any protocol */
        hints.ai_canonname = NULL;
        hints.ai_addr = NULL;
        hints.ai_next = NULL;
  
        int r;
        if ((r = getaddrinfo(host, port, &hints, &ad_info)) != 0) {
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
          errno_throw<connection_error>("socket() failed");
        }
        
        if (connect(socket_, server.address(), server.address_len()) == -1) {
          errno_throw<connection_error>("connect() failed");
        }
        
        COMMON_DEBUG_MESSAGE("Got sockets.");
      }
      
      //! \brief Disconnects
      ~connection_base() {
#ifdef LRCON_WINDOWS
        closesocket(socket_);
#else 
        close(socket_);
#endif
      }
      
    public:
      //! Necessary to be public for the message types to call it but not the user.
      int socket() { return socket_; }
  };
  
  const int wait_for_select_error = -1;
  const int wait_for_select_timeout = 0;
  
  //! 0 if timeout, time_left if no timeout, -1 if error.  Time params are offsets.
  inline int wait_for_select(int socket_fd, int timeout_usecs = 1000000) {
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(socket_fd, &rfds);
        
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = timeout_usecs;
    if (select(socket_fd+1, &rfds, NULL, NULL, &timeout) == -1) {
      return wait_for_select_error; // don't throw because the callers might need a different exception
    }
    
    if (FD_ISSET(socket_fd, &rfds)) {
      return timeout.tv_usec; 
    }
    else {
      return wait_for_select_timeout;
    }
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
  static T server_to_native_endian(T x) {
    return x;
  }
  
  template <typename T>
  static T native_to_server_endian(T x) {
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
  
  //! For convenience and readaility
  const int read_type_error = -1;
  const int send_type_error = -1;
  
  /*! 
  \brief Reads an integral type, ensureing endianness.
  */
  template<typename T>
  int read_type(int socket, T &v) {
    int bytes = recv(socket, (char *) &v, sizeof(T), 0);
    v = server_to_native_endian(v);
    return bytes;
  }
  
  //! Get a value from a buffer.  Also increments idx by reference
  template<typename T>
  T from_buffer(const void *source, size_t &idx) {
    T v;
    char *s = (char *) source+idx;
    common::endian_memcpy(v, s);
    idx += sizeof(T);
    return v;
  }
  
  //! Adds to a string (also increments idx by reference).  The compiler definitely should 
  //! optimise the by-value copy out when initialising a variable.
  inline std::string from_buffer(void *buf, size_t &idx, size_t max) {
    char *b = (char *) buf;
    size_t start = idx;
    while (idx < max && b[idx++] != '\0');
    size_t chrs = idx - start;
    return std::string(&b[start], chrs);
  }
  
  
  /*! 
  \brief Convenience wrapper, ensuring endianness.
  \returns bytes sent or -1 on error
  */
  template <typename T>
  bool send_type(int socket, T v) {
    v = native_to_server_endian(v);
    int bytes = send(socket, (char *) &v, sizeof(T), 0);
    return bytes;
  }
  
  template <typename Exception>
  int read_to_buffer(int socket_fd, void *buff, size_t buffsz, const char *errormsg = "recv() failed") {
    int read = 0;
#ifdef LRCON_WINDOWS
    if ((read = recv(socket_fd, (char *) buff, buffsz, 0)) == -1) {
#else 
    if ((read = recv(socket_fd, buff, buffsz, 0)) == -1) {
#endif
      common::errno_throw<Exception>(errormsg);
    }
    return read;
  }
  
  inline int read_to_buffer(int socket_fd, void *buff, size_t buffsz, const char *errormsg = "recv() failed") {
    return read_to_buffer<recv_error>(socket_fd, buff, buffsz, errormsg);
  }
}

#endif

