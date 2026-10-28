#ifndef joedb_main_wrapper_declared
#define joedb_main_wrapper_declared

#include "joedb/ui/Arguments.h"

#include <iosfwd>

namespace joedb
{
 int main_wrapper(int (*main)(Arguments &), int argc, char **argv);

 int main_wrapper
 (
  int (*main)(Arguments &, std::istream &, std::ostream &, std::ostream &),
  int argc,
  char **argv
 );
}

#endif
