#ifndef DEBUG_UTIL_HPP_sdfgasgq
#define DEBUG_UTIL_HPP_sdfgasgq

#include <netdb.h>
#include <iostream>

//! Debug an addrinfo structure.
inline void addrinfo_debug(struct addrinfo *a) {
  std::cout << "  ai_flags: " << a->ai_flags << " ";
  if (AI_PASSIVE & a->ai_flags) std::cout << "AI_PASSIVE ";
  if (AI_CANONNAME & a->ai_flags) std::cout << "AI_CANONNAME ";
  if (AI_NUMERICHOST & a->ai_flags) std::cout << "AI_NUMERICHOST ";
  if (AI_V4MAPPED & a->ai_flags) std::cout << "AI_V4MAPPED ";
  if (AI_ADDRCONFIG & a->ai_flags) std::cout << "AI_ADDRCONFIG ";
  std::cout << std::endl;
  
  std::cout << "  ai_family: " << a->ai_family;
  if (a->ai_family == AF_INET) {
    std::cout << " (IPv4)" << std::endl;
  }
  else if (a->ai_family == AF_INET6) {
    std::cout << " (IPv6)" << std::endl;
  }
  else if (a->ai_family == AF_UNSPEC) {
    std::cout << " (AF_UNSPEC)" << std::endl;
  }
  /*
  same as socket's domain param.
  
      PF_UNIX, PF_LOCAL   Local communication              unix(7)
      PF_INET             IPv4 Internet protocols          ip(7)
      PF_INET6            IPv6 Internet protocols          ipv6(7)
      PF_IPX              IPX - Novell protocols
      PF_NETLINK          Kernel user interface device     netlink(7)
      PF_X25              ITU-T X.25 / ISO-8208 protocol   x25(7)
      PF_AX25             Amateur radio AX.25 protocol
      PF_ATMPVC           Access to raw ATM PVCs
      PF_APPLETALK        Appletalk                        ddp(7)
      PF_PACKET           Low level packet interface       packet(7)
  */
  else {
    std::cout << "  (?)" << std::endl;
  }
  std::cout << "  ai_socktype: " << a->ai_socktype;
  if (a->ai_socktype == SOCK_DGRAM) {
    std::cout << " (UDP)" << std::endl;
  }
  else if (a->ai_socktype == SOCK_STREAM) {
    std::cout << " (TCP)" << std::endl;
  }
  else {
   std::cout << " (?)" << std::endl;
  }
  std::cout << "  ai_protocol: " << a->ai_protocol << " ";
  struct protoent *pr = getprotobynumber(a->ai_protocol);
  if (pr == NULL) {
    perror("getprotobynumber()");
  }
  else {
    std::cout << "(" << pr->p_name << ")" std::endl;
  }
  
  std::cout << "  ai_addrlen: " << a->ai_addrlen << std::endl;
  // a->ai_addr; <-- can't debug this... ?
  if (a->ai_flags & AI_CANONNAME) std::cout << "  ai_canonname: '" << a->ai_canonname << "'" << std::endl;
}


inline void addrinfo_list_debug(struct addrinfo *a) {
  struct addrinfo *a = ad_info;
  size_t i = 1;
  while (a != NULL) {
    std::cout << "Address #" << i << std::endl;
    addrinfo_debug(a);
    a = a->ai_next;
    ++i;
  }
  std::cout << "Finished printing addresses." << std::endl;
}

#endif 
