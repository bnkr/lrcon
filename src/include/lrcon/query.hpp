/*!
\file
\brief The public query interface to a server.

http://developer.valvesoftware.com/wiki/Server_Queries

This paradoxically seems to be a much more complicated protocol than RCON.

Steam uses a packet size of 1400 bytes + IP/UDP headers. If a request or response 
needs more packets for the data it starts the packets with an additional header in 
the following format: 

\verbatim
Data          Type          Comment  
Type          long          -2 (0xFFFFFFFE) to indicate that data is split over several packets 
Request ID    long          The server assigns a unique number to each split packet. (Source 
                            Engine only) If the top bit of the request ID is set then this 
                            packet has been compressed with the BZip2 algorithm before being 
                            split, after you get all the pieces you need to uncompress it with 
                            BZ2_bzBuffToBuffDecompress() before reading the raw data. 
Packet Number byte / short  (Half-Life 1)The lower four bits represent the number of packets (2 
                            to 15) and the upper four bits represent the current packet starting 
                            with 0. 
                            (Source Engine) The lower byte is the number of packets, the upper 
                            byte this current packet starting with 0. 
Size of the Splits   
             short         (Source Engine)For the new version of the source engine we changed the 
                           SPLITPACKET header slightly to include at the end a short which is the size 
                           of the splits (it used to be hard coded). In our games a user can request 
                           a smaller packet size before a split occurs, the default is 0x04e0 which 
                           is 1248 bytes. The query protocols, however, don't allow requesting 
                           a different split size, so the new default (1248) is what you should always 
                           see there. So if you are querying a TF2 server you need to account for 
                           that. A TF2 server op can also lower the max routable payload, so you can't 
                           guarantee that it'll always be 0x04e0.
\endverbatim

If the packet is compressed, there are two extra fields in the first packet. 
\verbatim
Data   Type       Comment 
  Type   integer    Number of bytes the data uses after decompression; 
  Type   integer    CRC32 checksum of the uncompressed data for validation.
\endverbatim

Because UDP packets can arrive in different order or delayed, every packet should be verified by 
its Request ID and the Packet Number! Every request or response with or without this header is 
continued with an integer -1 (0xFFFFFFFF) and the user data.

So by reading the first 4 bytes you can decide whether the data is split (-2) or in one packet (-1).

\par Queries

- \b A2A_PING
  Ping the server to see if it exists, this can be used to calculate the latency to the server.
- \b A2S_SERVERQUERY_GETCHALLENGE
  Returns a challenge number for use in the player and rules query. 
- \b A2S_INFO
  Basic information about the server. 
- \b A2S_PLAYER
  Details about each player on the server. 
- \b A2S_RULES
  The rules the server is using.

\par A2A_PING

\b Request

\verbatim
  Data      Type    Value 
  Heading   byte    'i' (0x69)
\endverbatim

\b Reply

\verbatim
  Goldsource servers
    Data      Type      Value 
    Heading   byte      'j' (0x6A) 
    Content   string    Null 
  Source servers
    Data      Type      Value 
    Heading   byte      'j' (0x6A) 
    Content   string    '00000000000000'
\endverbatim

\par A2S_SERVERQUERY_GETCHALLENGE

\b Request 

Challenge values are required for A2S_PLAYER and A2S_RULES requests, you can use this request to get one. 

\verbatim FF FF FF FF 57 \endverbatim

\b Reply

\verbatim
Data        Type    Comment 
Type        byte    Should be equal to 'A' (0x41) 
Challenge   long    The challenge number to use
\endverbatim

\par A2S_INFO

\b Request

\verbatim
FF FF FF FF 54 53 6F 75 72 63 65 20 45 6E 67 69   ÿÿÿÿTSource Engi
6E 65 20 51 75 65 72 79 00                        ne Query
\endverbatim

i.e. -1 (int), 'T' (byte), "Source Engine Query" (string)

\b Reply

\verbatim
Data                Type    Comment 
Type                byte    Should be equal to 'I' (0x49) 
Version             byte    Network version. 0x07 is the current Steam version. 
Server Name         string  The Source server's name, eg: "Recoil NZ CS Server #1" 
Map                 string  The current map being played, eg: "de_dust" 
Game Directory      string  The name of the folder containing the game files, eg: "cstrike" 
Game Description    string  A friendly string name for the game type, eg: "Counter Strike: Source" 
AppID               short   Steam Application ID 
Number of players   byte    The number of players currently on the server 
Maximum players     byte    Maximum allowed players for the server 
Number of bots      byte    Number of bot players currently on the server 
Dedicated           byte    'l' for listen, 'd' for dedicated, 'p' for SourceTV 
OS                  byte    Host operating system. 'l' for Linux, 'w' for Windows 
Password            byte    If set to 0x01, a password is required to join this server 
Secure              byte    if set to 0x01, this server is VAC secured 
Game Version        string  The version of the game, eg: "1.0.0.22" 
Extra Data (EDF)    byte    if present this specifies which additional data fields will be included 
if ( EDF & 0x80 )   short   The server's game port # is included 
if ( EDF & 0x40 )   short, string   The spectator port # and then the spectator server name are included 
if ( EDF & 0x20 )   string  The game tag data string for the server is included [future use]
\endverbatim

There are lots of different types for different servers here.

\par A2S_PLAYER

\b Request

\verbatim FF FF FF FF 55 <4 byte challenge number> \endverbatim

\b Reply

The players response has two sections, the initial header: 

\verbatim
Data          Type    Comment 
Type          byte    Should be equal to 'D' (0x44) 
Num Players   byte    The number of players reported in this response 
\endverbatim

Then for each player the following fields are sent: 

\verbatim
 Data           Type     Comment 
Index           byte     The index into [0.. Num Players] for this entry 
Player Name     string   Player's name 
Kills           long     Number of kills this player has 
Time connected  float    The time in seconds this player has been connected
\endverbatim

\par A2S_RULES

\b Request

\verbatim FF FF FF FF 56 <4 byte challenge number> \endverbatim 

\b Reply

The rules response has two sections, the initial header: 

\verbatim 
Data        Type     Comment 
Type        byte     Should be equal to 'E' (0x45) 
Num Rules   short    The number of rules reported in this response 
\endverbatim 

Then for each rule the following fields are sent: 

\verbatim 
Data        Type    Comment 
Rule Name   string  The name of the rule 
Rule Value  string  The rule's value
\endverbatim 

<hr>

\par Basics/Certainties

- we have the send/recv. thing therefore it makes sense to have a send(command, return)
  function.  Another reason of course is that we can deal with out of order packets 
  easier.
- this can be implemented easily in the rcon interface.
- sends are all easy, except challenges which require a small ammount of state.

\par Problems

- what about ensureing sequencing?  I don't know how to implement that well.  Well, I do
  but it's hard!  Think about sliding window etc etc.

\par Guessing

- could do types again?  eg, send(rules_query(chal_num)).  Pretty decent way I believe.
  - hard to do return values tho, without having a big list of send(T1, T2)... which
    I suppose is actually ok.
- returning values?  It's a binary proto, clearly so we cant' do basic return like that.
  - I think the best way is to convert it with send(cmd, rcv) where rcv is a big packet
  - would send() do the converting, or the rcv?
  - cmd would be doing the converting to send, so for consistancy rcv might do the 
    conversion into proper data.  Also makes sense because rcv deffo should do its own
    validation.
- data structures.
  - mostly it's very easy to deal with.  A few vectors... maybe some templates for the 
    containers... nothing stressful.


\par Use Cases

\code
connection con(host("whatever"));
ping myping;
con.send(myping); // use the send type as the return
myping.latency();
\endcode

This one is likely the best.  The entire thing is a request/response thing so I 
think it makes the most sense.  You have to have two hierachies the other way.
It conflicts with teh design of the rcon proto tho.

\code
connection conn;
ping myping(conn);
myping.latency();
\endcode


\todo Write here a use case for each thingy.

*/

#ifndef QUERY_HPP_ifzth5xs
#define QUERY_HPP_ifzth5xs

#include "common.hpp"

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
  class query_base {};
  
  
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


