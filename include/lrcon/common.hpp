// Copyright (C) 2008 James Weber
// Under the LGPL3, see COPYING
/*!
\file
\brief Containing the entire \link common \endlink namespace.
*/

#ifndef COMMON_HPP_y3dykfiu
#define COMMON_HPP_y3dykfiu

// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netdb.h>

// #include <config.h>

/// stuffs on win vs. unix: http://tangentsoft.net/wskfaq/articles/bsd-compatibility.html

/// \todo WSAGetLastError() is needed to get errno; also the actual numbers are different (but gai_strerror would work 
///       (not threadsafe aparently))

/// \todo Stop using LRCON_WINDOWS and use WIN32 instead.

#ifdef WIN32
#define LRCON_WINDOWS
#endif 

#ifdef LRCON_WINDOWS
// there is no getaddrinfo etc. until winxp [later there might be portable wrappers in mingw - 
// there is one in vlc now, maybe could port?]
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
#  include <fcntl.h>
#endif

#ifdef HAVE_ENDIAN_H
#include <endian.h>
#endif

#include <cassert>
#include <cerrno>
#include <cstring>

#include <stdexcept>
#include <iostream>


#if defined(COMMON_DEBUG_MESSAGES) || defined(RCON_DEBUG_MESSAGES ) \
    || defined(QUERY_DEBUG_MESSAGES) 
#  include <iostream>
#  define LRCON_DEBUG_MESSAGE(x__)\
   std::cout << __FUNCTION__ << "(): " << x__ << std::endl;
#else
#  define LRCON_DEBUG_MESSAGE(x__)
#endif

#if defined(COMMON_DEBUG_MESSAGES)
#  define COMMON_DEBUG_MESSAGE(x__) LRCON_DEBUG_MESSAGE(x__)
#else
#  define COMMON_DEBUG_MESSAGE(x__)
#endif

/*!
\def RCON_SYS_LITTLE_ENDIAN  

Readability variable for endianness feature test.  When it is defined, the byte 
order is swapped transparently.
*/

/// \todo Use boost's endianism detection file (see in libbdbg).

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
    std::string m(message);
    m += ": ";
    m += strerror(errno);
    throw Exception(m);
  }
  
#ifdef LRCON_WINDOWS
  namespace {
    /*!
    \todo Probably there should be a way for the user to do this manually,
          so they can check for the exceptions themselves.  (Also this 
          would result in multiple definitions issues sometimes.)
    */
    struct network_init {
      WSADATA w;
    
      network_init() {
        int error = WSAStartup(0x0202, &w);
        COMMON_DEBUG_MESSAGE("WSAStartup done.  Code: " << error);
        
        if (error) {
          /// \todo better exception needed
          throw connection_error("Winsock error; couldn't run winsock startup.");
        }
        
        if (w.wVersion != 0x0202) { // wrong WinSock version!
          WSACleanup();
          /// \todo better exception needed
          throw connection_error("Wrong winsock version.");
        }
      }
      
      ~network_init() {
        WSACleanup();
      }
    };
    
    network_init winsock_stuff;
  }
  
  
#endif
  
  
  /*!
  \brief Take care of DNS and so on.  Prefer to use sub-classes.

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
        COMMON_DEBUG_MESSAGE("Host is: " << host << ":" << port << " attr:" << attr);
        
        if (host == NULL) throw std::invalid_argument("host not be empty");
        
        if (port == NULL) throw std::invalid_argument("port must be a numeric string");
        
        assert(! (attr & tcp & udp));
        
        struct addrinfo hints;
#ifdef LRCON_WINDOWS
        // Necessary or gai crashes with unrecoverable error during database lookup.  
        // (But a swear I set all the fields, mum!)
        ZeroMemory(&hints, sizeof(struct addrinfo)); 
#endif
        
        hints.ai_family = AF_INET; /*AF_UNSPEC*/;     /* values: IPv6 AF_INET or AF_INET6 */
        
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
  
        COMMON_DEBUG_MESSAGE("Getting address info.");
        int r;
        if ((r = getaddrinfo(host, port, &hints, &ad_info)) != 0) {
          throw connection_error(std::string("getaddrinfo() failed: ") + gai_strerror(r));
        }
        
        if (ad_info->ai_addr == NULL || ad_info->ai_addrlen == 0) {
          throw connection_error("No socket address returned.");
        }
#if defined(COMMON_DEBUG_MESSAGES)
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
  
  
  const int wait_for_select_timeout = 0;
  typedef enum {wait_readable, wait_writeable} wait_for_select_mode_t;
  
  //! 0 if timeout, time_left if no timeout, -1 if error.  Time params are offsets.
  inline int wait_for_select(int socket_fd, wait_for_select_mode_t mode = wait_readable, int timeout_usecs = 1000000) {
    
    /// \todo This func implies that I should really have stored the connection object
    ///       in the command object.  It breaks things atm, but later I should 
    ///       organise it like that, and move this function in there (if I haven't
    ///       already done it in the new network lib)
    ///       
    ///       Update: actually, this function could be called as a member of connection.
    ///       There's not a problem with it really, I just get the socket in the command
    ///       things so I can write to it directly.  In fact, I could really do everything
    ///       using a read/write member function of connection.
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(socket_fd, &fds);
        
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = timeout_usecs;
    int ret;
    if (mode == wait_readable) {
      ret = select(socket_fd+1, &fds, NULL, NULL, &timeout);
    }
    else {
      ret = select(socket_fd+1, NULL, &fds, NULL, &timeout);
    }
    
    if (ret == -1) {
      errno_throw<connection_error>("select() failed");
    }
    
    if (FD_ISSET(socket_fd, &fds)) {
      return timeout.tv_usec; 
    }
    else {
      return wait_for_select_timeout;
    }
  }
  
  //! \brief Non-instancable base class which resolves a circular dependancy from having authing.
  class connection_base {
    static const int wait_for_select_timeout = 0;
    
    int socket_;
    
    protected:
      /*! 
      \brief Connects to the server.
      */
      connection_base(const host &server) {
        COMMON_DEBUG_MESSAGE("Initialising sockets.");
        socket_ = ::socket(server.family(), server.type(), 0);
        if (socket_ == -1) {
          errno_throw<connection_error>("socket() failed");
        }
        
#ifndef LRCON_WINDOWS
        COMMON_DEBUG_MESSAGE("Setting nonblock.");
        
        int flags = fcntl(socket_, F_GETFL, 0);
        if (flags == -1) {
          errno_throw<connection_error>("fcntl(): F_GETFL failed");
        }
        else if (! (flags & O_NONBLOCK)) {
          if (fcntl(socket_, F_SETFL, flags | O_NONBLOCK) == -1) { 
            std::cerr << "warning: couldn't set non-blocking flags on the socket.  "
                         "If the host drops packets, the connection will block forever." << std::endl;
          }
        }
        
        COMMON_DEBUG_MESSAGE("Connecting socket.");
        int ret = connect(socket_, server.address(), server.address_len());
        if (errno == EINPROGRESS && ret == -1) {
          COMMON_DEBUG_MESSAGE("Connect is now in progress.");
        }
        else {
          errno_throw<connection_error>("connect() failed");
        }
        
        COMMON_DEBUG_MESSAGE("Waiting for select.");
        if (wait_for_select(socket_, wait_writeable, 1000000) == wait_for_select_timeout) {
          /// \todo Coluld do with a timeout_error.
          throw connection_error("timeout when connecting to host.");
        }
        
        socklen_t option_value_size = sizeof(int);
        int option_value;
        if (getsockopt(socket_, SOL_SOCKET, SO_ERROR, (void*)(&option_value), &option_value_size) < 0) { 
          errno_throw<connection_error>("checking for socket error with getsockopt() failed");
        } 
        assert(option_value_size == sizeof(option_value));
        
        if (option_value) { 
          errno = option_value;
          errno_throw<connection_error>("delayed connection failed");
        } 
        
        COMMON_DEBUG_MESSAGE("Setting blocking again.");
        flags = fcntl(socket_, F_GETFL, 0);
        if (flags == -1) {
          errno_throw<connection_error>("fcntl(): F_GETFL failed");
        }
        else if (flags & O_NONBLOCK) {
          if (fcntl(socket_, F_SETFL, flags & ~O_NONBLOCK) == -1) { 
            errno_throw<connection_error>("could not reset the socket to blocking mode");
          }
        }
        
#else 
        /// \todo Get this working on windows
        ///       http://www.codeguru.com/forum/showthread.php?t=312668 - could help
        
        COMMON_DEBUG_MESSAGE("Connecting socket.");
        if (connect(socket_, server.address(), server.address_len()) == -1) {
          errno_throw<connection_error>("connect() failed");
        }
#endif
        COMMON_DEBUG_MESSAGE("Sockets all set up.");
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
  
  
  /// \todo These functions are a mess!  Stick with:
  ///       - read_to_buffer (should be in connection)
  ///       - send_from_buffer (should be in connection)
  ///       - var_from_buffer = read_type, but from a buffer
  ///       - var_to_buffer = send_type but to a buffer
  /// 
  /// This is all used by query tho.
  /// 
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
}

#endif

