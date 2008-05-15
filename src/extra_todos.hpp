// Copyright (C) 2008 James Weber
// Under the GPL3, see COPYING
/*!
\file
\brief File to add doxygen todos.

\todo Better doxygen support in configure:
      - Add configure --enable-docs[=file], default=Doxyfile.  make all would make
        it in this case.  Also needs a clean-local target.
      - Add docs/ to the docdir install target (if enabled).

\todo An option should be provided to turn on and off debug messages in configure.

\todo Make timeouts dynamic.
    - class timeout 
    - default one in conn, which can be changed by the user
    - otherwise one is given in the constructors of objects.

\todo Re-organise exceptions to accomodate query

\todo After query is done, look at if it's possible to share some of the reading
      code (well, as much of it as possible actually)

\todo Look at the possiblity of making a network lib!!  Ostream based networking 
      would pwn to such a massive degree.

\todo I'm not sure about my method of warnings.  It's a bit bad to be printing out
      things in a library.  Much nicer to give the user a way to check for them,
      and these assertion type things can be turned off.

\todo Investigate making this work on windows.  The API actually seems pretty 
      similar.

\todo For unit testing I need to write a program where I can use nc in order to
      send binary data to the program.  Something like writebin hex_values...
      and on stdout it will write.  Then writebin | nc whatever.  Not sure how
      to make it return the data tho without also writing a server...  

\todo It would be cheaper to make a lookup table to convert the endianness.  I only
      need it for int32 --- actually only for certain values too (request_id values,
      command_id values).

\todo I need to test split packets of RCON.  I really don't know how it is organised.
      Perhaps I could invent an eventscript which would return a massive ammount 
      of data (where massive >= max_payload).

\todo I should test fringe values for the sizes of the strings, because there's a bad
      performance case when read_data == max_data but no more packets are coming.  It's
      quite unlikely, but I should test anyway.  Hopefully the server will actually say 
      something about it.
*/
