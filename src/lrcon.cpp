// Copyright (C) 2008 James Weber
// Under the GPL3, see COPYING
/*!
\file 
\brief Client application to execute rcon commands.

\internal

\todo Interactive session... I suppose not much point really except to implement
      ctrl+delete and stuff like that.

\todo More options, eg don't print any output -- especially relevant for a scripted
      session.
*/

#include <lrcon/rcon.hpp>

#include <cstdlib> // exit_failure etc.
#include <unistd.h> // isatty

//! Run one command and handle errors. >0 on error.
int single_command(rcon::connection &conn, const std::string &command);
//! Run multiple commands from an istream.  >0 on error.
int stream_command(rcon::connection &conn, std::istream &in, const std::string &host, const std::string &port);

void print_usage(const char *pname) {
  std::cout 
      << pname << " -p password [OPTIONS] command [args]...\n"
      "Executes command with args on an RCON server and retrieve the output.  Command\n"
      "can be a dash (-) to trigger reading from stdin.  The program automatically\n" 
      "reads from stdin if it was redirected.\n\n"
      "  -p  password (required argument)\n"
      "  -P  port (default: 27015)\n"
      "  -s  server (default: localhost)\n"
      "  -h  this message and exit.\n\n"
      "lrcon Copyright (C) 2008 James Weber\n"
      "This program comes with ABSOLUTELY NO WARRANTY.  This is free software, and you\n"
      "are welcome to distribute it under the terms of the GPLv3.\n"
      << std::flush;
}

bool check_required_arg(int argc, const char * const argv[], int i, const char *arg) {
  if (i >= argc || argv[i][0] == '-') {
    std::cerr << "Error: option " << arg << " requires an argument." << std::endl;
    print_usage(argv[0]);
    return false;
  }
  else {
    return true;
  }
}

//! Read a list of commands from some stream
int stream_command(rcon::connection &conn, std::istream &in, const std::string &host, const std::string &port) {
  std::string cmd;
  do {
    std::getline(in, cmd);
    if (cmd == "") continue;
    
    std::cout << host << ":" << port << " > rcon " << cmd << std::endl;
    if (int r = single_command(conn, cmd)) return r;
  } while (! in.eof());
  
  return EXIT_SUCCESS;
}

int single_command(rcon::connection &conn, const std::string &command) {
  try {
    rcon::command cmd(conn, command);
    if (cmd.data().length() == 0 || cmd.data()[cmd.data().length() - 1] != '\n') {
      std::cout << cmd.data() << std::endl;
    }
    else {
      std::cout << cmd.data() << std::flush;
    }
  }
  catch (rcon::error &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int main(const int argc, const char *const argv[]) {
  const char *host = "localhost";
  const char *port = "27015";
  const char *pass = "";
  std::string command;
  bool read_from_stdin = false;

  {
    int i = 1;
    while (i < argc) {
      if (strcmp(argv[i], "-h") == 0) {
        print_usage(argv[0]);
        return 0;
      }
      else if (strcmp(argv[i], "-p") == 0) {
        ++i;
        if (! check_required_arg(argc, argv, i, "-p")) return EXIT_FAILURE;

        pass = argv[i];
      }
      else if (strcmp(argv[i], "-P") == 0) {
        ++i;
        if (! check_required_arg(argc, argv, i, "-P")) return EXIT_FAILURE;

        port = argv[i];
      }
      else if (strcmp(argv[i], "-s") == 0) {
        ++i;
        if (! check_required_arg(argc, argv, i, "-s")) return EXIT_FAILURE;
        
        host = argv[i];
      }
      else if (strcmp(argv[i], "-") == 0) {
        read_from_stdin = true;
      }
      else if (*argv[i] != '-') {
        // end of arguments
        break;
      }
      ++i;
    }
    
    read_from_stdin = read_from_stdin || ! isatty(fileno(stdin));
    
    if (pass[0] == '\0') {
      std::cerr << "Error: no password parameter." << std::endl;
      print_usage(argv[0]);
      return EXIT_FAILURE; 
    }
    
    if (i >= argc && ! read_from_stdin) {
      std::cerr << "Error: no command given." << std::endl;
      print_usage(argv[0]);
      return EXIT_FAILURE;
    }
    else {
      try {
        rcon::connection conn(rcon::host(host, port), pass);
        
        if (read_from_stdin) {
          if (int r = stream_command(conn, std::cin, host, port)) return r;
        }
        else {
          std::string command = argv[i++];
          while (i < argc) {
            command += " ";
            command += argv[i++];
          }
          std::cout << host << ":" << port << " > rcon " << command << std::endl;
          if (int r = single_command(conn, command)) return r;
        }
      }
      catch (rcon::error &e) {
        std::cerr << "Connection failure: " << e.what() << std::endl;
        return EXIT_FAILURE;
      }
    }
  }

  return EXIT_SUCCESS;
}

