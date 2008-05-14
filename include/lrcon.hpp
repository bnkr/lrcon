// Copyright (C) 2008 James Weber
// Under the LGPL3, see COPYING
/*!
\file
\brief Includes the entire lrcon library.
*/

/*!
\mainpage

\section s_intro Introduction

\b lrcon is a library for communication with RCON servers and steam query
servers.  Also included are some simple programs to serve as examples
though they can be used as applications in their own right.

Refer to the 'related pages' tab to see documentation for seperate parts
of the library.  If you're not using doxygen to read this, all page 
documentation resides in \link lrcon.hpp \endlink.  

\section s_versions Version History

\par April 2008
\par 0.1 
- Basic interface for RCON and the protocol code.

\par 0.2 
- Rewritten interface, but the protocol code is the same.
- Basics of the query interface.

\par 0.3
- Program for running a single RCON command on a server.

\par 0.4 
- Interface compatible reorganisation of some code to accomodate query
  connections.

\par May 2008
\par 0.4.1
- Optimised use of timeouts (ie, getting rid of most of them) in RCON.
  The RCON/lrcon part is considered 'good' now.
- Improvments to lrcon app.

\section s_plans Plans

\par 0.5 
- First hacked up functionality for the query part.
- Application for querying a counter-strike: source server.

\par 0.6
- Reorganisation of query code to reduce duplication.
- Dynamic timeouts.

\par 1.0
- Resolve outstanding todos and tidying.

\par 1.x
- Querying things other than cs:source.
- RCON to things other than source engine servers.
*/

/*!
\page p_RCON Rcon Usage

\section s_rcon_intro Introduction

The following documentation shows the usage of the RCON library by example.

All components are in the \link rcon \endlink namespace, included by 
\link rcon.hpp \endlink.

\note The protocol is little endian, so conversion is done transparently
      for big endian hosts.

\subsection ss_rcon_basic_usage Basic Usage

The following example connects to a server, authenticates, runs the 'status' 
command and prints the data the server sends back.  It should be noted that
much more specific error checking can be done.

\include rcon_basic_usage.cpp

\subsection ss_rcon_advanced Advanced Features

This example does the same function but also features retries and does not
immediatly authenticate with the server.

\include rcon_deferred_authorisation.cpp

\todo document timeouts here when I implemented them.

\note Connection objects may be temporary.

*/

/*!
\page p_query Query Usage

\todo Document this and have examples once it's done.  It's basically the same but meh.


\see http://developer.valvesoftware.com/wiki/Server_Queries for documentation of the
     server query protocol.

*/

/*!
\page p_RCON_proto The Rcon Protocol

\section s_rcon_proto_reimplementation RCON Protocol Implementation

C++ is probably not a particularly useful language for this to be in so here are some 
very quick notes on re-implementing it.

\subsection ss_rcon_message_format Message Format

- int32 packet size -- size of the data \b not including the packet size int32.
- int32 request id  -- for sequencing, and sometimes used as a return value.
- int32 command     -- SERVERDATA_EXECCOMMAND = 2 / SERVERDATA_AUTH = 3, 
                       or SERVERDATA_RESPONSE_VALUE = 0 / SERVERDATA_AUTH_RESPONSE = 2
- string s1 -- is the command to run or return data.
- string s2 -- always null for send, carries data in a return. 

Strings are 4096 bytes max and null-terminated.  Data may be split over multiple 
packets.

\subsection ss_rcon_conversation An Example Conversation

- open tcp stream to the server
- send a packet with: 
  - command = SERVERDATA_AUTH 
  - request id = some constant
  - s1 = the password
  - s2 = null
- receive a packet:
  - command == SERVERDATA_AUTH_RESPONSE
  - request id == the constant to show auth success, -1 for rejected.  Anything 
    else error.
- now you're authed, send commands.
  - command = SERVERDATA_EXECCOMMAND
  - request id = anything
  - s1 = whichever command you will run
  - s2 = null
- return data may span multiple packets.  You must wait for a timeout in order
  to know that the data is finished.
- the return data's request id should always mirror the one you sent.  If it is
  -1 and the command_id is SERVERDATA_AUTH_RESPONSE, you have been kicked off.
  If it simply doesn't match, then there is some kind of error.

\subsection ss_rcon_notes Notes

- it seems that when authing, two packets are sent in acknowledgement.  The second is
  the real valid one.  This does not seem to be documented anywhere though.  (note:
  the conclusion here is that to be safe you should always read until a timeout, and you
  should always refresh the header values).  See the documentation for 
  auth_command::get_reply() for a more detailed list of the behavior of this.
- the entire protocol is little endian, not network order.  Basically, everything is
  x86 specific.
- always include both strings, even if one is null (so there will be two nulls at the 
  end normally)
- it seems that the header parts are duplicated across multi-packet responses, however
  this was not always the case.
- if you do not auth a connection, then a null packet is returned, but the request id 
  is still mirrored.

\see http://developer.valvesoftware.com/wiki/Source_RCON_Protocol for documentation 
     on the RCON protocol.

*/

#include <lrcon/query.hpp>
#include <lrcon/rcon.hpp>
