try {
  rcon::host h("127.0.0.1", "27015");
  rcon::connection conn(h);
  
  const char *pass = "password";
  size_t retries = 3;
  rcon::auth_command::auth_t auth_value = rcon::auth_command::error;
  do {
    rcon::auth_command returned(conn, pass, rcon::auth_command::nocheck);
    auth_value = returned.auth();
    
    if (retries == 0) {
      std::cerr << "Authorisation retries limit reached:\n" << returned.data() << std::endl;
      break;
    }
    else if (auth_value == rcon::auth_command::failed) {
      std::cerr << "Authorisation was denied:\n" << returned.data() << std::endl;
      break;
    }
    --retries;
  }
  while (auth_value == rcon::auth_command::error);
  
  retries = 3;
  while (retries--) {
    rcon::command returned(conn, "status", rcon::command::nocheck);
    if (! returned.auth_lost()) {
      std::cout << returned.data() << std::endl;
      break;
    }
    else {
      std::cerr << "There was an error: " << returned.data() << std::endl;
    }
  }

}
catch (rcon::connection_error &e) {
  std::cerr << "Unable to connect: " << e.what() << std::endl;
}
catch (rcon::proto_error &e) {
  std::cerr << "There was an error in protocol: " << e.what() << std::endl;
}
catch (rcon::recv_error &e) {
  std::cerr << e.what() << std::endl;
}
catch (rcon::send_error &e) {
  std::cerr << e.what() << std::endl;
}
catch (rcon::error &e) {
  std::cerr << e.what() << std::endl;
}
