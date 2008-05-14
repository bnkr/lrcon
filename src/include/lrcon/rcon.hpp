/*!
\file
\brief Includes all components for communicating with an RCON server.

See \ref p_RCON "RCON Protocol" for usage.

\internal

\todo Write unit tests.  Especially invalid data -- I don't handle it really (there 
      isn't a great deal of it tho).

\todo Make more assertions about the 'useless' properties of messages.  Could help
      diagnosis when the proto changes.

\todo It would be cheaper to make a lookup table to convert the endianness.  I only
      need it for int32 --- actually only for certain values too (request_id values, 
      command_id values).

\todo Test with other RCON server types.

\todo Buffering the data makes the program seem slow.  Is there a way we can ostream it
      maybe?

\todo Better error messages -- maybe do some string getting... strerror, gai_error etc.

\todo The payload return always has a newline at the end of it.  I should get rid of 
      this so that I can guarantee that it won't be newline terminated.  Also document
      this.  Should happen in the command base classes.

\todo Is it allowed for the server to send null return data?  If so, then I can check
      for this error --- if so it means that there was no authorisation given.

\todo some code can be shared with query, especially the memcpy stuff and endian crap.
      Or at least I think so.
*/


#ifndef RCON_HPP_58dx55q1
#define RCON_HPP_58dx55q1

#include "common.hpp"

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <cassert>

#include <string>
#include <stdexcept>
#include <iostream>


#ifdef RCON_DEBUG_MESSAGES 
#  include <iostream>
#  define RCON_DEBUG_MESSAGE(x__)\
   std::cout << __FUNCTION__ << "(): " << x__ << std::endl;
#else
#  define RCON_DEBUG_MESSAGE(x__)
#endif

//! All components of the RCON library.
namespace rcon {
#ifdef RCON_SYS_LITTLE_ENDIAN
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
  static T rcon_to_native_endian(T x) {
    return swap_endian(x);
  }
  
  template <typename T>
  static T native_to_rcon_endian(T x) {
    return swap_endian(x);
  }
#endif
  
  //! Catch-all error class.
  struct error : public std::runtime_error {
    error(const std::string &s) : std::runtime_error(s) {}
    ~error() throw() {}
  };
  
  //! Network error, indicating some kind of failure.
  struct network_error : public rcon::error {
    network_error(const std::string &s) : error(s) {}
    ~network_error() throw() {}
  };
  
  //! General communications errors which might be recoverable.
  struct comm_error : public rcon::error {
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
  

  //! Convenience wrapper class
  struct host : public common::host {
    //! \param is_ip  means no lookup will be done if true
    host(const char *host, const char *port, bool is_ip = false) 
    : common::host(host, port, ((is_ip) ? (common::host::is_ip|common::host::tcp) : common::host::tcp)) {}
  };
  

  /*! 
  \brief Non-instanciable base containing the implementation of the packet format.
  
  All values returned by accessors are in host order.  All values given should also
  be in host order -- all conversions are handled automatically.
  */
  class command_base {
    protected:
      //! Values sent in the packet and returned by the server as the command id.
      typedef enum {
        auth_request = 3, 
        auth_response = 2, 
        exec_request = 1, 
        exec_response = 0
      } command_id_t;
      
      int32_t send_request_id_;
      int32_t recvd_request_id_;
      int32_t command_id_;
      std::string payload_;
    
    public:
      //! Maximum length of one of the string fields.
      static const size_t max_string_length = 4096;
      
      //! The complete string payload read in the response.
      const std::string &data() const { return payload_; }
    
    protected:
      //! \pre this->command_id_ is one of command_id_t
      command_id_t command_id() const { return static_cast<command_id_t>(command_id_); }
      int32_t receive_id() const { return recvd_request_id_; } 
      int32_t send_id() const { return send_request_id_; } 
      
      /*! 
      \brief Send the packet but does not get the reply; the subclass decides the receive strategy.
      
      \throws send_error 
      */
      command_base(common::connection_base &c, int32_t send_id, command_id_t command_id, const std::string &payload)
      : send_request_id_(send_id), command_id_((int32_t) command_id), payload_(payload) {
        write(c.socket());
      }
      
      /*! 
      \brief Read values for the data members from the socket.
      \throws recv_error      failures from recv() and erroneus timeouts.
      \throws response_error  the command id was not one of command_id_t

      \param first_read  causes an recv_error if there is a timeout.
      
      \returns \c true if there is more to read; \c false otherwise.
      */
      bool read(int socket, bool first_read = true) {
        RCON_DEBUG_MESSAGE("Reading a packet.");
        
        if (! common::wait_for_select(socket)) {
          RCON_DEBUG_MESSAGE("Timeout.");
          if (first_read) {
            throw recv_error("timed out before any data was read.");
          }
          else {
            return false;
          }
        }
        
        /// \todo we should be able to exit early if we read less than the 
        ///        max ammount of data... requires that the timeout checking 
        ///        is reorganised (do it at the end -- pre-connect stuff at 
        ///        the start should be done be done by the receiver... need
        ///        to be careful tho (what if you read exactly the right ammount?)
        ///        I suppose that's ok here -- you can just check that the two 
        ///        strings were read (inc. the null terminators).
        
        // Two ints, two strings.
        const size_t max_packet_size = sizeof(int32_t) * 2 + command_base::max_string_length * 2;
        // Two ints, two nulls.
        const size_t min_packet_size = sizeof(int32_t) * 2 + 1 + 1;
        
        int32_t size;
        if (! read_type<int32_t>(socket, size)) throw recv_error("recv() failed for size.");
        
        if ((size < min_packet_size) || (size > max_packet_size)) {
          throw recv_error("invalid data size.");
        }
        
        if (! read_type<int32_t>(socket, recvd_request_id_)) throw recv_error("recv() failed for request_id.");
          
        int32_t t;
        if (! read_type<int32_t>(socket, t)) throw recv_error("recv() failed for command_id.");
        
        RCON_DEBUG_MESSAGE("Properties of read: ");
        RCON_DEBUG_MESSAGE("  Initial size: " << size);
        RCON_DEBUG_MESSAGE("  Request id: " << recvd_request_id_);
        RCON_DEBUG_MESSAGE("  Command id: " << t);
        
        if (t == command_base::auth_request || t == command_base::auth_response ||
            t == command_base::exec_request || t == command_base::exec_response) {
          command_id_ = t;
        }
        else {
          throw response_error("received an invalid command id.");
        }
        
        {
          size -= 2 * sizeof(int32_t);
          const int max_payload_size = max_string_length * 2;
          assert(size < max_payload_size);
          assert(size > 0);
          char buf[max_payload_size];
          size_t idx = 0;
          RCON_DEBUG_MESSAGE("  Reading " << size << " from the socket for payload.");
          while (size > 0) {
            int bytes = -1;
            if ((bytes = recv(socket, &buf[idx], size, 0)) == -1) {
              perror("recv()");
              throw recv_error("recv() failed for stringdata.");
            }
            
            size -= bytes; 
            idx += bytes;
          }
          // there are always nulls at the end anyway.
          buf[max_payload_size-2] = buf[max_payload_size-1] = '\0';
          payload_.append(buf, idx-2); // don't put the 2 nulls on 
          
          RCON_DEBUG_MESSAGE("  Payload: '" << payload_ << "' (used " << idx << "/" << max_payload_size << " bytes)");
        }
        
        // there might be more to get
        return true;
      }
      
      //! \brief Reads all values ino the payload string until it reaches a timeout.
      void read_all(int sock) {
        payload_ = "";
        bool more = read(sock, true);
        while (more) {
          more = read(sock, false);
        }
      }
      
      //! Send *this as an RCON packet.
      void write(int socket) {
        // int32 + int32 + payload.length + null + null;
        int32_t size = 8 + payload_.length() + 1 + 1;
        
        RCON_DEBUG_MESSAGE("Data sending properties: ");
        RCON_DEBUG_MESSAGE("  Packet: " << size);
        if (! send_type<int32_t>(socket, size)) {
          perror("send()");
          throw send_error("error sending size.");
        }
        
        RCON_DEBUG_MESSAGE("  Request id: " << send_request_id_);
        if (! send_type<int32_t>(socket, send_request_id_)) {
          perror("send() failed:");
          throw send_error("error sending request_id.");
        }
        
        RCON_DEBUG_MESSAGE("  Command id: " << command_id_);
        if (! send_type<int32_t>(socket, command_id_)) {
          perror("send()");
          throw send_error("error sending command_id.");
        }
        
        RCON_DEBUG_MESSAGE("  Payload: '" << payload_ << "'");
        int bs;
        if ((bs = send(socket, payload_.c_str(), payload_.length() + 1, 0)) == -1) {
          perror("send()");
          throw send_error("error sending payload.");
        }

        if ((bs = send(socket, "", sizeof(char), 0)) == -1) {
          perror("send()");
          throw send_error("error sending terminating null.");
        }
      }
     
    private:
      //! Reads an integral type, ensureing endianness.
      //! \deprecated  Use the one in common.
      //! \returns false in case of error.
      template<typename T>
      bool read_type(int socket, T &v) {
        if (recv(socket, &v, sizeof(T), 0) == -1) {
          perror("recv()");
          return false;
        }
        else {
          v = rcon_to_native_endian(v);
          return true;
        }
      }
      
      //! Convenience wrapper, ensuring endianness.
      //! \returns false in case of error.
      template <typename T>
      bool send_type(int socket, T v) const {
        v = native_to_rcon_endian(v);
        if (send(socket, &v, sizeof(T), 0) == -1) {
          perror("recv()");
          return false;
        }
        else {
          return true;
        }
      }
  };
// #endif 
  

  /*! 
  \brief Command for authorisation.  
  
  All methods take and return host-endian values.  Conversion is transparent.
  */
  class auth_command : public command_base {
    public:
      //! Arbitrary value sent which the server will mirror back in an 'ok' response.
      static const int32_t auth_send_req_id = 10;
      
      //! Server returns this to say the password was bad.
      static const int32_t auth_denied_req_id = -1;
      
      //! Token type to use a non-validating constructor
      typedef enum {nocheck} nocheck_t;
      
      //! Returned by \link auth_command::auth() \endlink to denote the response state.
      typedef enum {success, failed, error} auth_t;
     
      /*! 
      \brief Send an authentication request and validate the return.
      
      \pre strlen(password) < 4096.
      \pre request_id != \link rcon::auth_command::auth_denied_req_id \endlink
      
      \throw bad_password  if authorisation was rejected 
      \throw auth_error    if an error happened (you could send the password again in this case)
      \throw send_error    
      \throw recv_error 
      \throw proto_error   The server returned something other than an auth response.
      
      \post \link auth() \endlink == \link rcon::auth_command::success \endlink
      */
      auth_command(common::connection_base &conn, const std::string &password, 
                   int32_t request_id = auth_send_req_id)
      : command_base(conn, request_id, command_base::auth_request, password) {
        RCON_DEBUG_MESSAGE("Initialising an RCON auth request: '" << password << "'");
        assert(request_id != auth_denied_req_id);
        assert(password.length() < 4096);
        get_reply(conn);
        
        if (auth() == failed) {
          throw bad_password("authentication denied.");
        }
        else if (auth() == error) {
          throw auth_error("the server returned an unexpected value.");
        }
      }
      
      /*! 
      \brief Authenticate, but do not ensure success (you must test authed()).
      
      \pre strlen(password) < 4096.
      \pre request_id != auth_denied_req_id
      
      \throws send_error
      \throws recv_error
      \throws proto_error
      */
      auth_command(common::connection_base &conn, const std::string &password, 
                   nocheck_t, int32_t request_id = auth_send_req_id)
      : command_base(conn, request_id, command_base::auth_request, password) {
        RCON_DEBUG_MESSAGE("Initialising an RCON auth request (no checking): '" << password << "'");
        assert(request_id != auth_denied_req_id);
        assert(password.length() < 4096);
        get_reply(conn);
      }
      
      //! \brief Check the request ids match.
      auth_t auth() {
        if (receive_id() == auth_denied_req_id) {
          return failed;
        }
        else if (receive_id() == send_id()) {
          return success;
        }
        else {
          return error;
        }
      }
      
    private:
      /*!
      I make the following undocumented assumption:
      - authing returns a 'mirror' packet followed by the actual auth accepted/denied packet.  
        (Actually the implementation would be broken if I didn't assume this!) 
      - There will be exactly two packets.  If there are more than two packets, further calls 
        will probably fail.
      - The mirror packet will be a standard valid response header (ie, send_id == recieve_id 
        and command_id == exec_response) but the data will be null (I only warn if there is 
        null data).
      - 
      
      \throw proto_error  if my assumptions were wrong.
      */
      void get_reply(common::connection_base &conn) {
        payload_ = "";
        read(conn.socket(), true);
        if (receive_id() != send_id() || command_id() != command_base::exec_response) {
          throw proto_error("request ID was not returned by the server."); /// \todo should it be revc_error?
        }
        
#ifdef RCON_DEBUG_MESSAGE
        if (payload_ != "") {
          RCON_DEBUG_MESSAGE("Warning: the server's mirror packet has some data with it: '" 
              << payload_ << "' (" << payload_.length() << " bytes)");
        }
#endif 
        
        read(conn.socket(), true);
        if (command_id() != command_base::auth_response) {
          throw proto_error("the server did not return an authorisation response.");
        }       
        
#ifdef RCON_DEBUG_MESSAGE
        if (payload_ != "") {
          RCON_DEBUG_MESSAGE("Warning: the server's auth response packet has some data with it: '" 
              << payload_ << "' (" << payload_.length() << " bytes)");
        }
#endif 
        
      }
  };

  /*! 
  \brief An arbitrary RCON command.  
  
  All methods take and return host-endian values.  Conversion is transparent.
  */
  class command : public command_base {
    public:
      typedef enum {nocheck} nocheck_t;
      
      //! Arbitrary number to default to
      static const int32_t default_request_id = 42;
      
      /*! 
      \brief Initialise and check for validity
      
      \pre request_id != \link auth_command::auth_denied_req_id \endlink.
      
      \param command     text of the command to send
      \param send_id     an arbitrary value which will be echoed back to the client except 
                         under error conditions.
      
      \throws auth_error      if the server returned a bad auth message (kicking us off)
      \throws revc_error      any data read error
      \throws response_error  the server did not mirror the request id and it was not a failed auth.
      \throws send_error      any send error
      
      \note You should avoid making the request id == \link auth_command::auth_send_req_id \endlink for
            safety, however it is not required.
      
      \post \link valid() \endlink == true
      \post \link auth_lost() \endlink == false
      */
      command(common::connection_base &conn, const std::string &command, int32_t send_id = default_request_id)
      : command_base(conn, send_id, command_base::exec_request, command) {
        RCON_DEBUG_MESSAGE("Initialising an RCON command: '" << command << "'");
        get_reply(conn);
        
        if (auth_lost()) {
          throw auth_error("authentication was lost.");
        }
        else if (! valid()) {
          throw response_error("request ids did not match.");
        }
      }
      
      //! \brief Don't throw exceptions if the server responded badly (except 'real' errors).
      command(common::connection_base &conn, const std::string &command, nocheck_t, int32_t send_id = default_request_id)
      : command_base(conn, send_id, command_base::exec_request, command) {
        RCON_DEBUG_MESSAGE("Initialising an RCON command: '" << command << "'");
        assert(send_id != auth_command::auth_denied_req_id);
        get_reply(conn);
      }
      
      //! Checks the request id was mirrord back correctly.
      bool valid() const { return send_id() == receive_id(); }
      
      //! If a auth denied response was recieved or the command id recieved was an auth response.
      bool auth_lost() const { 
        return receive_id() == auth_command::auth_denied_req_id 
               || command_id() == command_base::auth_response;
      }
      
    private:
      //! Single-read
      void get_reply(common::connection_base &conn) {
        payload_ = ""; /// \todo I should proabably have a state reset function instead
        read(conn.socket(), true); // error on timeout
      }
  };
  

  //! An authenticated connection.
  class connection : public common::connection_base {
    public:
      /*! 
      \brief Connects to the server and auths.
      */
      connection(const host &server, const char *password) : common::connection_base(server) {
        RCON_DEBUG_MESSAGE("Initialising authed connection.");
        auth_command a(*this, password);
      }
      
      /*! 
      \brief Initialises a non-authorised connection.  
      
      Use this if you want to send the auth_command manually (perhaps to view its 
      output).  The connection is valid to send messages to.
      
      \warning  The protocol documentation implies that the server will return 
                authorisation errors if you send packets before authorising, but 
                it does not!  Therefore you must be very careful to send the auth 
                command.  It seems to return a packet with null data.
      */
      connection(const host &server) : common::connection_base(server) {
        RCON_DEBUG_MESSAGE("Initialising connection with no authing.");
      }
      
    protected:
      //! override the access.
      int socket() { return connection_base::socket(); }
  };
}


#endif // RCON_HPP_58dx55q1

