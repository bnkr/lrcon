// Copyright (C) 2008 James Weber
// Under the GPL3, see COPYING
/**
\file
\brief The public query app.

\todo Work out how to deal with out of order packets. 

\todo move common code into query_base and all over the place

\par Problems
- The connection code seems identical
  - first I need to resolve this bug that's in the rcon one where it blocks forever.  
  - Also, there's the prolblem of friend classes.  I don't really want to have both parts as
    friends.  
  - What I could do instead is make a small wrapper class with protected inheritance 
    for rcon and query, meaning the socket is not public.  Each module then has its
    wrapper class make a friend declaration.
- having said all that, I need to deal with out of order packets!  This might need a very
  different design
  - could work around it by simply saying that only one transaction can be completed at
    a time (in other words, don't have pipelineing).
  - would need a packet buffer for every connection
  - how do I determine the last packet?
  - how do I determine packet sequence number?  It's not made clear at all.
  - where would OOO packets be dealt with?  Connection, return object?  
    - If we do it in the query object then pipelining could happen (theoretically at least, 
      providing the request id is seperate from the sequence number.)
    - doing it in the connection is problematic becuase you have to read the packet format
      (which rightly belongs in the query)
    - so it is resolved that it gets done in the query object.
- code reuse: seems that some code from query_base is duplcated in command_base.  Not much 
  tho.
- timeouts: sometimes we know how many packets, or at least the user might.  We do not want
  to have to wait for packets every time!  So the todo

*/




#include <lrcon/rcon.hpp>
#include <lrcon/query.hpp>
#include "print_buf.hpp"

#include <cstdio>
#include <iostream>
void print_endian(int bs, size_t sz = sizeof(int)) {
  std::cout << "bytes: " << sz << std::endl;
  char *p = (char*) &bs;
  size_t i = 0;
  printf("%d as bytes:\n", bs);
  while (i < sz) {
    printf("%02X ", (p[i++]&255));
  }
  printf("\n");
}


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <cstring>
#include <ctime>
#define trc(thing__) std::cout << thing__ << std::endl;

             void prthex(char c) {
printf("%c %02X\n", c, c);
             }
             


namespace query {
  namespace {
    //! Complete ping packet
    const unsigned char pkt_ping[] = {
      0xFF, 0xFF, 0xFF, 0xFF, 
      0x69
    }; 
    
    //! Complete info packet
    const unsigned char pkt_info[] = {
      0xFF, 0xFF, 0xFF, 0xFF, 
      0x54, 0x53, 0x6F, 0x75,
      0x72, 0x63, 0x65, 0x20,
      0x45, 0x6E, 0x67, 0x69,
      0x6E, 0x65, 0x20, 0x51,
      0x75, 0x65, 0x72, 0x79,
      0x00
    };
    
    //! Complete challenge packet
    const unsigned char pkt_challenge[] = {
      0xFF, 0xFF, 0xFF, 0xFF,
      0x57
    };
    
    //! Add a 4 byte challenge reply for the real packet.
    const unsigned char pkt_players_header[] = {
      0xFF, 0xFF, 0xFF, 0xFF,
      0x55
    };
    
    //! Add a 4 byte challenge reply for the real packet.
    const unsigned char pkt_rules_header[] = {
      0xFF, 0xFF, 0xFF, 0xFF,
      0x56
    };
    
    
    //! Helper function to send some arbitrary static data.
    inline int send_buffered_packet(int socket, const unsigned char *pkt, size_t sz) {
      int sent_bytes;
      if ((sent_bytes = send(socket, pkt, sz, 0)) == -1) {
        perror("send()");
        throw std::runtime_error("send() failed"); /// \todo proper except
      }
      return sent_bytes;
    }
  }
  
  //! Common properties of static packets (ie, single static send buffer)
  class static_packet : public query_base {
     /// atm it seems this is a bit useless

  };
  
  //! Common properties of dynamic packets (ie, send a header and some data)
  class dynamic_packet : public query_base {
    /// atm it seems this is a bit ueeless.. too much function wrapping is just confusing.
  };
  
  //! \brief A ping command to measure latency.
  /// 
  /// \todo Somehow I might have to organise an error-tollerant connection in order that it doesn't
  ///       throw if the host doesn't exist... since I haven't actually worked out how to check this
  ///       yet it's moot tho ^^
  class ping : public static_packet {
    int latency_;
    
    public:
      static const int timeout = -1;
      
      ping(common::connection_base &conn) : latency_(timeout) {
        QUERY_DEBUG_MESSAGE("Sending ping packet.");
        send_buffered_packet(conn.socket(), pkt_ping, sizeof(pkt_ping));
        
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(conn.socket(), &rfds);
            
        const int timeout_wait = 500000;
        struct timeval time;
        time.tv_sec = 0;
        time.tv_usec = timeout_wait;
        if (select(conn.socket()+1, &rfds, NULL, NULL, &time) == -1) {
          perror("select()");
          throw std::runtime_error("select"); /// \todo proper exception
        }
        
        // time.usec has the remaning time left
        if (! FD_ISSET(conn.socket(), &rfds)) {
          latency_ = timeout;
        }
        else {
          latency_ = timeout_wait - time.tv_usec; 
        }
                
        /// I don't actually need to read this stuff do I?  Certainly don't need to process it.
        
        read(conn.socket());
      }
      
      //! Did the server reply?
      bool pingable() const { return latency_ != -1; }
      
      /*! 
      \brief The latency value calculated in nano-seconds (usec)
       
      \note Most systems don't provide usec accuracy.  It would be better get msec instead.
      */
      int latency() const { return latency_; }
      int latency_ms() const { return latency_ / 100; }
    
    protected:
      bool read(int socket_fd) { 
        //! \todo duplicated reading conversion from buff->val here.
        
        // We don't actually care about the return value here, just that 
        // it exists.
        
        int read = 0;
        char buf[1400];
        if ((read = recv(socket_fd, buf, 1400, 0)) == -1) {
          perror("recv()");
          throw int(1); /// \todo proper except
        }
        
        char reply = buf[4];
        if (reply != 'j') {
          std::cerr << "Warning: invalid ping reply received." << std::endl;
        }

        const char *ptr = &(buf[4+2]);
        if (buf[0] == '\0') {
          QUERY_DEBUG_MESSAGE("Detected a goldsrc server.");
        }
        else if (strncmp(ptr, "0000000000000", 14) == 0) {
          QUERY_DEBUG_MESSAGE("Detected a source server.");
        }
        else {
          std::cerr << "Warning: invalid ping reply received." << std::endl;
        }
        
        /// \todo somehow check that more data wasn't given
        
//         print_buf((const unsigned char*)buf, read); 
        
        return true;
      }
  };
  
  /// question: do you always need a new challenge number for every relevant request?
  
  /// Extensibility: other games have query protocols.  We could well implement them using
  /// the same data structures, but the actual send/recv would need to be abstracted.  I
  /// guess this is for later, but the best solution is probably to have a ping_data
  /// class and a source_server_ping class.  The specific class takes the data class and
  /// assigns its members.  Might be better to extend an abstract ping tho.  Better for
  /// initialisation.  Might involve virtual functions tho... certainly would reduce 
  /// code duplication.
  
  //! \brief Query for server name, num players etc.
  class info : public static_packet {
    public:
      info(common::connection_base &conn) {
        QUERY_DEBUG_MESSAGE("Sending info packet.");
        send_buffered_packet(conn.socket(), pkt_info, sizeof(pkt_info));
        read_all(conn.socket());
      }
      
    protected:
      /// this function could be generalised for any bytesequence
      bool read(int socket, bool first_read = false) {
//         using common::read_type;

        if (! common::wait_for_select(socket)) {
          if (first_read) throw int(1);
          return false;
        }
        
        int read = 0;
        char buf[1400];
        if ((read = recv(socket, buf, 1400, 0)) == -1) {
          perror("recv()");
          throw int(1); /// \todo proper except
        }
        
        print_buf((const unsigned char *)buf, read);
        
        /// \todo verify the read size; specifically that it doesn't end before the required fields are done 
        ///       (prolly repeated checking).
        
        /// \todo deal with split packets... somehow?!  Would require breaking in the middle... maybe I 
        ///       could put the whole thing in a loop... what happens if it runs out of data in the middle 
        ///       of a string tho?
        
        QUERY_DEBUG_MESSAGE("Properties of read:");
        size_t idx = 0;
        int32_t split_type;
        common::endian_memcpy(split_type, &buf[idx]);
        enum split_type {split_single = -1, split_multiple = -2};
        if (split_type == split_single) {
          QUERY_DEBUG_MESSAGE("  split type: single");
          /// \todo how to handle this?
        }
        else if (split_type == split_multiple) {
          QUERY_DEBUG_MESSAGE("  split type: multiple");
          /// \todo Handle somehow
        }
        else {
          throw std::runtime_error("split type invalid");
        }
        
        idx += sizeof(int32_t);
        
        int8_t packet_type;
        common::endian_memcpy(packet_type, &buf[idx]);
        if (packet_type != 0x49) throw std::string("wrong packet type flag"); /// \todo proper except
        QUERY_DEBUG_MESSAGE("  packet type: '" << (char) packet_type << "'");
        idx += sizeof(int8_t);
        
        int8_t steam_version;
        common::endian_memcpy(steam_version, &buf[idx]);
        QUERY_DEBUG_MESSAGE("  steam version: " << (int) steam_version);
        idx += sizeof(int8_t);
        
        {
          int start = idx;
          while (idx < read && buf[idx++] != '\0'); // idx will be on the first char of the next field
          size_t chrs = idx - start;
          std::string server_name(&buf[start], chrs);
          QUERY_DEBUG_MESSAGE("  server name: " << server_name);
        }
        
        {
          int start = idx;
          while (idx < read && buf[idx++] != '\0');
          size_t chrs = idx - start;
          std::string map(&buf[start], chrs);
          QUERY_DEBUG_MESSAGE("  map: " << map);
        }

        {
          int start = idx;
          while (idx < read && buf[idx++] != '\0');
          size_t chrs = idx - start;
          std::string dir_name(&buf[start], chrs);
          QUERY_DEBUG_MESSAGE("  game dir: " << dir_name);
        }
        
        {
          int start = idx;
          while (idx < read && buf[idx++] != '\0');
          size_t chrs = idx - start;
          std::string long_name(&buf[start], chrs);
          QUERY_DEBUG_MESSAGE("  long string: " << long_name);
        }
        
        int16_t steam_app_id;
        common::endian_memcpy(steam_app_id, &buf[idx]);
        QUERY_DEBUG_MESSAGE("  steam_app_id: " << (int) steam_version);
        idx += sizeof(int16_t);
        
        int8_t num_players;
        common::endian_memcpy(num_players, &buf[idx]); 
        QUERY_DEBUG_MESSAGE("  num_players: " << (int) num_players);
        idx += sizeof(int8_t);
        
        int8_t max_players;
        common::endian_memcpy(max_players, &buf[idx]); 
        QUERY_DEBUG_MESSAGE("  max_players: " << (int) max_players);
        idx += sizeof(int8_t);
        
        int8_t num_bots;
        common::endian_memcpy(num_bots, &buf[idx]); 
        QUERY_DEBUG_MESSAGE("  num_bots: " << (int) num_bots);
        idx += sizeof(int8_t);
        
        int8_t server_type;
        common::endian_memcpy(server_type, &buf[idx]);
        if (server_type == 'l') {
          QUERY_DEBUG_MESSAGE("  server_type: listen");
        }
        else if (server_type == 'd') {
          QUERY_DEBUG_MESSAGE("  server_type: dedicated");
        }
        else if (server_type == 'p') {
          QUERY_DEBUG_MESSAGE("  server_type: sourcetv");
        }
        else {
          throw std::string("bad server type.");
        }
        idx += sizeof(int8_t);
        
        int8_t os_type;
        common::endian_memcpy(os_type, &buf[idx]);
        if (os_type == 'l') {
          QUERY_DEBUG_MESSAGE("  os_type: linux");
        }
        else if (os_type == 'w') {
          QUERY_DEBUG_MESSAGE("  os_type: windows");
        }
        else {
          throw std::string("bad server os type.");
        }
        idx += sizeof(int8_t);
        
        bool password;
        common::endian_memcpy(password, &buf[idx]); 
        QUERY_DEBUG_MESSAGE("  passworded: " << (int) password);
        idx += sizeof(int8_t);
        
        bool vac;
        common::endian_memcpy(vac, &buf[idx]); 
        QUERY_DEBUG_MESSAGE("  vac: " << (int) vac);
        idx += sizeof(int8_t);
        
        {
          size_t start = idx;
          while (idx < read && buf[idx++] != '\0');
          size_t chrs = idx - start;
          std::string game_vers(&buf[start], chrs);
          QUERY_DEBUG_MESSAGE("  game version: " << game_vers);
        }
        
        int8_t extra_data;
        common::endian_memcpy(extra_data, &buf[idx]); 
        QUERY_DEBUG_MESSAGE("  extra_data_flag: " << (int) extra_data);
        idx += sizeof(int8_t);
        
        if (extra_data & 0x80) {
          int16_t portno;
          common::endian_memcpy(portno, &buf[idx]); 
          QUERY_DEBUG_MESSAGE("  port num: " << portno);
          idx += sizeof(int16_t);
        }
        if (extra_data & 0x40) {
          int16_t spect_portno;
          common::endian_memcpy(spect_portno, &buf[idx]); 
          QUERY_DEBUG_MESSAGE("  spectator port no: " << spect_portno);
          idx += sizeof(int16_t);
          
          {
            size_t start = idx;
            while (idx < read && buf[idx++] != '\0');
            size_t chrs = idx - start;
            std::string spect_server_name(&buf[start], chrs);
            QUERY_DEBUG_MESSAGE("  spect_server_name: " << spect_server_name);
          }
        }
        if (extra_data & 0x20) {
          {
            size_t start = idx;
            while (idx < read && buf[idx++] != '\0');
            size_t chrs = idx - start;
            std::string game_tag(&buf[start], chrs);
            QUERY_DEBUG_MESSAGE("  game_tag: " << game_tag); 
          }
        }
        
        
        /// \todo put the fields into data members and give them accessors
        
        /// here it would be best if I try and return before having to try a timeout.  I guess 
        /// if I read les than 1400 bytes then I can exit.  Otherwise, I have no choiec to do
        /// do it (the problem is if I read exactly 1400 bytes and that's the end -- there is
        /// no "I'm ending now" packet (afaik).  Have to check that tho.  Might be able to work
        /// it out if there is a multi-packet header.  Maybe the last packet would have a -1 code?
        /// Have to test this stuff.
        
        
        return true;
      }
      
      /// if I put this in the parent and call it from the self::constructor would
      /// it still work?
      void read_all(int socket) {
        /// this is totally duplicated from rcon (but it might be expanded to deal with
        /// multipackets)
        bool more = read(socket, true);
        while (more) {
          more = read(socket, false);
        }
      }
  };

  /// \todo maybe I should not bother doing endian conversion on the challenge num and simply say, 
  ///       that  this class is not to be used.
  //! \brief Get the challenge number for use in players and rules queries.
  class challenge : public static_packet {
    int32_t challenge_num_;
    
    public:
      challenge(common::connection_base &conn) {
        QUERY_DEBUG_MESSAGE("Challenge query");
        send_buffered_packet(conn.socket(), pkt_challenge, sizeof(pkt_challenge));
        
        /// \todo what about getting more than one packet back?
        read(conn.socket());
      }
      
      //! This will always be in little endian order.
      int32_t challenge_num() {
        return challenge_num_;
      }

    protected:
      void read(int socket) {
        QUERY_DEBUG_MESSAGE("Reading:");
        
        if (! common::wait_for_select(socket)) throw std::runtime_error("timed out reading"); ///\todo proper execpt
        
        /// this func should really be designed more like the others (for consistancy)
        
        /// \todo use a constant for the max steam packet length
        char buff[1400];
        int bytes = recv(socket, buff, 1400, 0);
        if (bytes == -1) throw int(1);
        else if (bytes != sizeof(int32_t) + sizeof(int8_t) + sizeof(int32_t)) {
          throw std::runtime_error("response error -- sent invalid packet");
        }
        
        int32_t type;
        common::endian_memcpy(type, &buff[0]);
        QUERY_DEBUG_MESSAGE("  type: " << type);
        int8_t flag;
        common::endian_memcpy(flag, &buff[4]);
        QUERY_DEBUG_MESSAGE("  flag: " << (char) flag);
        if (flag != 0x41) throw std::runtime_error("response error -- bad packet flag");
        memcpy(&this->challenge_num_, &buff[5], sizeof(int32_t)); // don't convert endianness
        QUERY_DEBUG_MESSAGE("  challenge_num: " << challenge_num_);
        
        /// \todo poll the socket here -- if there's data then it's an error (but would we need to
        ///       timeout anyway?  Polling would always end up returning.)  It would be a lot faster
        ///       to assume that only one packet is returned.
          
        if (common::wait_for_select(socket)) std::cerr << "Warning: too much data received." << std::endl;
      }
  };
  
  /// \todo consider a class static_packet and challenged_packet query objects.
  
  
  
  //! \brief List of players on the server
  class players : public dynamic_packet {
    /// this class is identical to rules
    
    int32_t challenge_no_;
    
    public:
      players(common::connection_base &conn) {
        QUERY_DEBUG_MESSAGE("Sending players request.");
        challenge c(conn);
        challenge_no_ = c.challenge_num();
        write(conn.socket());
        read(conn.socket());
      }
      
    protected:
      void write(int socket) {
        QUERY_DEBUG_MESSAGE("Sending players request");
        unsigned char sendbuff[sizeof(pkt_players_header) + sizeof(int32_t)];
        memcpy((void *)sendbuff, pkt_players_header, sizeof(pkt_players_header));
        memcpy(&sendbuff[sizeof(pkt_players_header)], &challenge_no_, sizeof(int32_t));
        send_buffered_packet(socket, sendbuff, sizeof(sendbuff));
      }
      
      void read(int socket) {
        /// \todo need to organise multipackets here.
        
        QUERY_DEBUG_MESSAGE("Receiving players data:");
        char buff[1400];
        int bytes;
        do {
          bytes = recv(socket, buff, 1400, 0);
          if (bytes == -1) {
            perror("recv()");
            throw std::runtime_error("failed recv");
          }
          
          size_t idx = 0;
          int32_t split_type;
          common::endian_memcpy(split_type, &buff[idx]);
          //! \todo use constants for the split types
          if (split_type == -1) {
            QUERY_DEBUG_MESSAGE("  split type: packet");
          }
          else if (split_type == -2) {
            QUERY_DEBUG_MESSAGE("  split type: split");
            /// \todo handle split types (somehow!)
          }
          else {
            throw std::runtime_error("reieved invalid split type."); /// \todo proper execption
          }
          idx += sizeof(int32_t);
          
          int8_t type;
          common::endian_memcpy(type, &buff[idx]);
          QUERY_DEBUG_MESSAGE("  packet type: " << (char) type);
          if (type != 0x44) throw std::runtime_error("invalid packet type field"); /// \todo proper exception
          idx += sizeof(int8_t);
          
          // now a list of players
          while (idx < bytes) {
            /// \todo could I encapsulate this code somewhere?  Declare var, copy, advance index... it's very
            ///       necessary because it's easy to forget to advance the buffer pointer.
            int8_t player_num;
            common::endian_memcpy(player_num, &buff[idx]);
            idx += sizeof(int8_t);
            QUERY_DEBUG_MESSAGE("  player num: " << (int) player_num);
            
            ///! \todo This string reading code is also duplicated
            size_t start = idx;
            while (idx < bytes && buff[idx++] != '\0');
            size_t numchrs = idx - start;
            std::string player_name(&buff[start], numchrs);
            QUERY_DEBUG_MESSAGE("  player name: " << player_name);
            
            int32_t kills;
            common::endian_memcpy(kills, &buff[idx]);
            idx += sizeof(int32_t);
            QUERY_DEBUG_MESSAGE("  kills: " << kills);
                
            /// \todo How do I handle endianness?  How do I make it sure that it's a 32bit float?
            float connect_time;
            common::endian_memcpy(connect_time, &buff[idx]);
            idx += sizeof(float);
            QUERY_DEBUG_MESSAGE("  connect time: " << connect_time);
          }
          
          /// ok there's a problem here.  Seems that it just replies with the 
          /// challenge number again.  You can tell because the type is set to 0x41
          
          std::cout << "Message received (" << bytes << " bytes): " << std::endl;
          print_buf(buff, bytes);
        } while (common::wait_for_select(socket));
      }
  };
  
  /// \todo this is completely unfinished
  //! \brief List of some server vars.
  class rules : public dynamic_packet {
    int32_t challenge_no_;
    
    public:
      rules(common::connection_base &conn) {
        challenge c(conn);
        challenge_no_ = c.challenge_num();
        write(conn.socket());
        read(conn.socket());
      }
      
    protected:
      void write(int socket) {
        QUERY_DEBUG_MESSAGE("Sending rules request");
        unsigned char buff[sizeof(pkt_rules_header) + sizeof(challenge_no_)];
        memcpy(&buff[0], pkt_rules_header, sizeof(pkt_rules_header));
        memcpy(&buff[sizeof(pkt_rules_header)], &challenge_no_, sizeof(challenge_no_));
        int bytes = send_buffered_packet(socket, buff, sizeof(buff));
      }
      
      void read(int socket) {
        /// this is again a problem with multipackets.  How do I preserve which var I'm  writing to?
        /// What about reading it all into one big buffer and sorting it out later?  Out of order 
        /// packets problem tho...  Of course this is all asuming that hte data would just cut out
        /// and the fields are not aligned for you.  I need to test this stuff.  I guess the rules 
        /// one would be a good place to start -- probly more than 1400 bytes.  How does phprcon deal
        /// with it?  I guess maybe it doesnt seeing as it's so old.
        
        QUERY_DEBUG_MESSAGE("Receiving rules data");
        char buff[1400];
        int bytes;
        do {
          //! \todo this code for the packet header is duplicated virtually everywhere
          bytes = recv(socket, buff, 1400, 0);
          if (bytes == -1) {
            perror("recv()");
            throw std::runtime_error("failed recv");
          }
          
          //! \todo duplicated split header reading here
          size_t idx = 0;
          int32_t split_type;
          common::endian_memcpy(split_type, &buff[idx]);
          //! \todo use constants for the split types
          if (split_type == -1) {
            QUERY_DEBUG_MESSAGE("  split type: single");
          }
          else if (split_type == -2) {
            QUERY_DEBUG_MESSAGE("  split type: split");
            /// \todo handle split types (somehow!)
          }
          else {
            throw std::runtime_error("reieved invalid split type."); /// \todo proper execption
          }
          idx += sizeof(int32_t);
          
          int8_t type;
          common::endian_memcpy(type, &buff[idx]);
          idx += sizeof(int8_t);
          QUERY_DEBUG_MESSAGE("  packet type: " << (char) type);
          if (type != 0x45) throw std::runtime_error("wrong packet type received.");
          
          int16_t num_rules;
          common::endian_memcpy(num_rules, &buff[idx]);
          idx += sizeof(int16_t);
          QUERY_DEBUG_MESSAGE("  num rules: " << (int) num_rules);

          
          size_t start, numchrs;
          int rules_read = 0;
          while (idx < bytes) {
            /// \todo duplicated string reading code
            start = idx;
            while (idx < bytes && buff[idx++] != '\0');
            numchrs = idx - start;
            std::string key(&buff[start], numchrs);
            
            start = idx;
            while (idx < bytes && buff[idx++] != '\0');
            numchrs = idx - start;
            std::string value(&buff[start], numchrs);
            
            QUERY_DEBUG_MESSAGE("    " << key << " = " << value);
            ++rules_read;
          }
          
          if (rules_read != num_rules) std::cerr << "Warning: didn't read the right number of rules." << std::endl;
          
          
//           print_buf(buff, bytes);
        } while (common::wait_for_select(socket));
      }
  };
  
}


  







void do_a_ping() {
  query::connection conn(query::host("propperlush.net", "27015"));
  query::ping q(conn);
  std:: cout << "Ping latency: " << q.latency() << std::endl;
}


void do_an_info() {
  query::connection conn(query::host("propperlush.net", "27015"));
  query::info q(conn);
}

void do_a_challenge() {
  query::connection conn(query::host("propperlush.net", "27015"));
  query::challenge q(conn);
}

void do_a_players() {
  query::connection conn(query::host("propperlush.net", "27015"));
  query::players q(conn);
}

void do_a_rules() {
  query::connection conn(query::host("propperlush.net", "27015"));
  query::rules q(conn);
}
             
int main() {
  do_a_rules();
//   do_a_players();
//   do_a_challenge();
//   do_an_info();
  do_a_ping();
  
  
  
           
  return 0;
}



/***
ping proto notes:

I dunno why it gave me all this useless shit.  Seems it never gets used.

  // full size of each payload (not including UDP/IP headers).
  const size_t max_packet_size = 1400;
  
  Maybe this only happens when type == -2 ??

  struct steam_packet {
    // -2 if the packet is split, -1 means it is in one packet.
    int32_t type; 
    // Unique number for each split packet.  If the top bit is set then it 
    // is bzip2 compressed (source engine only).
    // What is the top bit?  Big endian, little endian?  Would I test the 
    // highest byte & '\0x01'?  Or maybe it means... gah I don't know.
    int32_t request_id; 
#ifdef SOURCE_ENGINE
    // The lower byte is the number of packets, the upper byte this current 
    // packet starting with 0
    int16_t packet_num;
#elif defined(GOLDSOURCE_ENGINE)
    // The lower four bits represent the number of packets (2 to 15) and the 
    // upper four bits represent the current packet starting with 0.
    int8_t packet_num;
#endif
    // Hardcoded to 1248 in goldsrc engines.
    int32_t split_size;
#ifdef SOURCE_ENGINE
    // Number of bytes the data uses after decompression.  Only present if 
    // compressed.
    int32_t decomp_bytes;   
    // CRC32 checksum of the uncompressed data for validation.  Only present 
    // if compressed.
    int32_t crc32;  
#endif

  };
  
  // "Because UDP packets can arrive in different order or delayed, every packet 
  // should be verified by its Request ID and the Packet Number!"
  
  // "Every request or response with or without this header is continued with an 
  // integer -1 (0xFFFFFFFF) and the user data.  So by reading the first 4 bytes 
  // you can decide whether the data is split (-2) or in one packet (-1)."
  // 
  // That implies:
  // - the data is in one packet if type == -1
  // - the data is in one packet if type == -2
  // - and a -2 implies there is no packet header...
  // - which in turn means that final response packets will have a -1.
  // 
  // That seems impossible because UDP might be out of order.  There must be a 
  // packet number, right?
  
  
*****/

