// Copyright (C) 2008 James Weber
// Under the LGPL3, see COPYING
/*!
\file
\brief Includes all components for communicating with an RCON server.

See \ref p_RCON "RCON Protocol" for usage.

\internal

\todo Test with other RCON server types.

\todo Buffering the data makes the program seem slow.  Is there a way we can ostream it
      maybe?  This is actually prety irrelevant after removing the timeouts.  The only
      problem is split packets where we could print out as we get it.  It also gives 
      the possiblity that we should make it easier to do this in the API -- the user
      code could know that it will receive a multi-packet sequence and then organise
      its timeouts accordingly.

\todo I could use common::read_to_buffer in here more I think.

\todo Would it be faster to read the entire lot into a buffer and then use endian_memcpy
      to sort it out?  Perhaps actually it's not such a problem because the packets get
      concatenated somehow in tcp... or do they... meh it's too late.
*/


#ifndef RCON_HPP_58dx55q1
#define RCON_HPP_58dx55q1

#include <lrcon/common.hpp>

#include <cassert>
#include <cstring>
#include <cerrno>

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
  static T rcon_to_native_endian(T x) {
    return swap_endian(x);
  }
  
  template <typename T>
  static T native_to_rcon_endian(T x) {
    return swap_endian(x);
  }
#endif
  
  //! For interface consistancy we alias these
  //@{
  typedef common::error error;
  typedef common::comm_error comm_error;
  typedef common::auth_error auth_error;
  typedef common::bad_password bad_password;
  typedef common::connection_error connection_error;
  typedef common::response_error response_error;
  typedef common::network_error network_error;
  typedef common::proto_error proto_error;
  typedef common::recv_error recv_error;
  typedef common::send_error send_error;
  typedef common::timeout_error timeout_error;
  //@}

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

      \param error_on_timeout  causes an recv_error if there is a timeout.
      
      \returns \c true if there is more to read; \c false otherwise.
      
      \warning The assumption is that there will only be more data if the entire 
               payload is full up.  This is not documented and actually rather
               hard to test since servers very rarely return split-packets.
      */
      bool read(int socket, bool error_on_timeout = true) {
        RCON_DEBUG_MESSAGE("Reading a packet.");
        
        int timeleft = common::wait_for_select(socket);
        if (timeleft == common::wait_for_select_timeout) {
          RCON_DEBUG_MESSAGE("Timeout.");
          if (error_on_timeout) {
            throw timeout_error("timed out before any data was read.");
          }
          else {
            return false;
          }
        }
        else if (timeleft == common::wait_for_select_error) {
          common::errno_throw<recv_error>("select() failed"); /// \todo different exception?
        }
        
        // Two ints, two strings.
        const size_t max_packet_size = sizeof(int32_t) * 2 + command_base::max_string_length * 2;
        // Two ints, two nulls.
        const size_t min_packet_size = sizeof(int32_t) * 2 + 1 + 1;
        
        int32_t size;
        if (common::read_type(socket, size) == common::read_type_error) {
          common::errno_throw<recv_error>("recv() failed for size");
        }
        
        if ((size < min_packet_size) || (size > max_packet_size)) {
          throw recv_error("invalid data size.");
        }
        
        if (common::read_type(socket, recvd_request_id_) == common::read_type_error) {
          common::errno_throw<recv_error>("recv() failed for request_id");
        }
          
        {
          // ensure t is actually a valid member of the enum before assigning it
          int32_t t;
          if (common::read_type(socket, t) == common::read_type_error) {
            common::errno_throw<recv_error>("recv() failed for command_id");
          }
          
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
        }
        
        const int max_payload_size = max_string_length * 2;
        size_t total_payload_size;
        {
          /// \todo use common::from_buffer here
          
          // Read the entire rest of the packet
          size -= 2 * sizeof(int32_t);
          
          assert(size < max_payload_size);
          assert(size > 0);
          char buf[max_payload_size];
          size_t idx = 0;
          RCON_DEBUG_MESSAGE("  Reading " << size << " from the socket for payload.");
          while (size > 0) {
            int bytes = -1;
            if ((bytes = recv(socket, &buf[idx], size, 0)) == -1) {
              common::errno_throw<recv_error>("recv() failed for stringdata");
            }
            
            size -= bytes; 
            idx += bytes;
          }
          
          // Concatinate the two strings value to the payload.
          buf[max_payload_size-2] = buf[max_payload_size-1] = '\0';
          payload_.append(buf);
          if (payload_.length() != idx-2) payload_.append(&buf[payload_.length()]);
          
          total_payload_size = idx;
          
          RCON_DEBUG_MESSAGE("  Payload: '" << payload_ << "' (used " << total_payload_size << "/" << max_payload_size << " bytes)");
        }
        
        // there might be more to get
        return (size == max_packet_size);
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
        if (common::send_type(socket, size) == common::send_type_error) {
          common::errno_throw<send_error>("error sending size");
        }
        
        RCON_DEBUG_MESSAGE("  Request id: " << send_request_id_);
        if (common::send_type(socket, send_request_id_) == common::send_type_error) {
          common::errno_throw<send_error>("error sending request_id");
        }
        
        RCON_DEBUG_MESSAGE("  Command id: " << command_id_);
        if (common::send_type(socket, command_id_) == common::send_type_error) {
          common::errno_throw<send_error>("error sending command_id");
        }
        
        RCON_DEBUG_MESSAGE("  Payload: '" << payload_ << "'");
        if (send(socket, payload_.c_str(), payload_.length() + 1, 0) == -1) {
          common::errno_throw<send_error>("error sending payload");
        }

        if (common::send_type(socket, '\0') ==  common::send_type_error) {
          common::errno_throw<send_error>("error sending terminating null");
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
  
  \internal
  
  \note When you do not auth first, the server will just return a null data packet.
        It is not determined whether we can use this to assert for an error though
        as it's possible some commands might not give any output.
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
        get_reply(conn, true);

      }
      
      //! \brief Don't throw exceptions if the server responded badly (except 'real' errors).
      command(common::connection_base &conn, const std::string &command, nocheck_t, int32_t send_id = default_request_id)
      : command_base(conn, send_id, command_base::exec_request, command) {
        RCON_DEBUG_MESSAGE("Initialising an RCON command: '" << command << "'");
        assert(send_id != auth_command::auth_denied_req_id);
        get_reply(conn, false);
      }
      
      //! Checks the request id was mirrord back correctly.
      bool valid() const { return send_id() == receive_id(); }
      
      //! If a auth denied response was recieved or the command id recieved was an auth response.
      bool auth_lost() const { 
        return receive_id() == auth_command::auth_denied_req_id 
               || command_id() == command_base::auth_response;
      }
      
    private:
      //! Will read multiple packets maybe
      void get_reply(common::connection_base &conn, bool check_validity = false) {
        payload_ = "";
        bool error_on_timeout = true;
        bool more = false;
        do {
          more = read(conn.socket(), error_on_timeout);
          
          if (check_validity) {
            if (auth_lost()) {
              throw auth_error("authentication was lost.");
            }
            else if (! valid()) {
              throw response_error("request ids did not match.");
            }
          }
          
          error_on_timeout = false;
        } while (more);
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

