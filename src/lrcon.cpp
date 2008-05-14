/*!
\file 
\brief Client application to execute rcon commands.

\internal

\todo  mode to a list of commands from stdin (ie, cat stuff)
- Possibly try matching error returns and stop the script.

\todo  Maybe upgrade to an interractive sesssion.
*/

#include <lrcon/rcon.hpp>

#include <cstdlib>

void print_usage(const char *pname) {
  std::cout 
      << pname << " -p password [OPTIONS] command [args]...\n"
      "Executes command with args on an RCON server and retreive the output.\n\n"
      "  -p  password (required argument)\n"
      "  -P  port (default: 27015)\n"
      "  -s  server (default: localhost)\n"
      "  -h  this message and exit.\n"
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

int main(const int argc, const char *const argv[]) {
  const char *host = "localhost";
  const char *port = "27015";
  const char *pass = "";
  std::string command;

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
      else if (argv[i][0] != '-') {
        // end of arguments
        break;
      }
      ++i;
    }
    
    if (pass[0] == '\0') {
      std::cerr << "Error: no password parameter." << std::endl;
      print_usage(argv[0]);
      return EXIT_FAILURE; 
    }
    
    if (i >= argc) {
      std::cerr << "Error: no command given." << std::endl;
      print_usage(argv[0]);
      return EXIT_FAILURE;
    }
    else if (*argv[i] == '-') {
      /// read commands from stdin
    }
    else {
      command = argv[i++];
      while (i < argc) {
        command += " ";
        command += argv[i];
        ++i;
      }
    }
    
    std::cout << host << ":" << port << " > rcon " << command << std::endl;
  }
  
  try {
    rcon::connection conn(rcon::host(host, port), pass);
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

