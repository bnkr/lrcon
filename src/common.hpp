#ifndef LRCON_COMMON_HPP
#define LRCON_COMMON_HPP

#if defined(QRCON_DEBUG_MESSAGES)
#  include <iostream>
#  define QRCON_DEBUG_MESSAGE(words___)\
   std::cout << "[qrcon:" << __FUNCTION__  << "] " << words___ << std::endl;
#else
#  define QRCON_DEBUG_MESSAGE(words__)
#endif

#endif
