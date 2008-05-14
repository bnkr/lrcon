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

*/
