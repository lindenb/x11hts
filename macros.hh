#ifndef X11HTS_MACROS
#define X11HTS_MACROS
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <cerrno>
#include "version.hh"

#define ASSERT_NOT_NULL(a) assert(a!=NULL)

#define FATAL(a) do {std::cerr <<"[FATAL][" << __FILE__<<":" << __LINE__ << "]" << a << std::endl;std::exit(EXIT_FAILURE);} while(0)
#define WARN(a) do {std::cerr <<"[WARN][" << __FILE__<<":" << __LINE__ << "]" << a << std::endl;} while(0)

#endif

