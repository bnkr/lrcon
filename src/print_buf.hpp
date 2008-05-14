/*!
\file

\todo This could go in texttools.

*/

#ifndef PRINT_BUF_HPP_33gp7bpp
#define PRINT_BUF_HPP_33gp7bpp

#include <cstdio>

//! Print an indivicual character
static inline void print_buf_chr(unsigned char c, char deadchar = '.') {
  if (c == '\n' || c == '\r' || c == '\t') {
    putchar(deadchar);
  }
  else if (c >= 0x20 && c <= 0x7E) {
    putchar(c);
  }
  else {
    putchar(deadchar);
  }
}

//! Print out hexdump of some data.
/// \todo make an ostream version (parameterised) but have an overload with the 
///       same interface for std::cout.  Will it print the right hexvalues?  I 
///       think so.
inline void print_buf(const void *buff, size_t size, char deadchar = '.') {
  size_t i = 0;
  const unsigned char *buf = (const unsigned char *)buff;
  while (i < size) {
    printf("%02X ", buf[i]);
    ++i;
    
    // Print the character representation of the buffer
    if ((i % 16) == 0) {
      printf("  ");
      size_t chrs = i - 16;
      while (chrs < i) print_buf_chr(buf[chrs++], deadchar);
      putchar('\n');
    }
    // Space after every 8 bytes.
    else if ((i % 8) == 0) {
      printf("  ");
    }
  }
  
  // if this isn't the end of a line we've got more stuff to print out
  if ((size % 16) != 0) {
    size_t printed = size % 16;
    size_t num_to_print = 16 - printed;
    i =  0;
    while (i < num_to_print) {
      if ((i % 8) == 0) {
        printf("  ");
      }
      printf("   ");
      ++i;
    }
    
    i = size - printed;
    while (i < size) print_buf_chr(buf[i++], deadchar);
  }
  putchar('\n');
}

#endif // #ifndef PRINT_BUF_HPP
