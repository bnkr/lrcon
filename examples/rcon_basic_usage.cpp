try {
  rcon::connection conn(rcon::host("example.com", "27015"), "password");
  rcon::command returned(conn, "status");
  std::cout << returned.data() << std::endl;
}
catch (rcon::error &e) {
  std::cerr << "Error: " << e.what() << std::endl;
}
