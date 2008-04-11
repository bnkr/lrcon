#include "../src/query.hpp"
#include "../src/rcon.hpp"

#define trc(thing) std::cout << thing << std::endl;

int main() {
  rcon::host h("localhost", "80");
  
  int socket_fd = socket(h.family(), h.type(), 0);
  if (socket_fd == -1) {
    perror("socket()");
    return 1;
  }
  
  trc(h.family());
  trc(h.type());
  trc(h.address());
  trc(h.address_len());
  
  if (connect(socket_fd, h.address(), h.address_len()) == -1) {
    perror("connect()");
    return 1;
  }
  return 0;
  const char *msg = "hello\n";
  if (send(socket_fd, msg, strlen(msg) + 1, 0) == -1) {
    perror("send()");
    return 1;
  }
  
  char buf[5000];
  if (recv(socket_fd, buf, 5000, 0) == -1) {
    perror("recv()");
    return 1;
  }
  buf[5000-1] = '\0';
  printf("Recvd:\n%s\n", buf);

  close(socket_fd);
  
  return 0;
}



