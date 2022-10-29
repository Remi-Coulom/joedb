#ifndef joedb_main_exception_catcher_declared
#define joedb_main_exception_catcher_declared

#include "joedb/Exception.h"

#include <iostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 inline void print_exception(const std::exception &e)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::cerr << "Error: " << e.what() << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 inline int main_exception_catcher
 ////////////////////////////////////////////////////////////////////////////
 (
  int (*main)(int, char**), int argc, char **argv
 )
 {
  try
  {
   return main(argc, argv);
  }
  catch (const std::exception &e)
  {
   print_exception(e);
   return 1;
  }
 }
}

#endif
