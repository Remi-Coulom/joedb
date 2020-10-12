#ifndef joedb_main_exception_catcher_declared
#define joedb_main_exception_catcher_declared

#include "joedb/Exception.h"

#include <iostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void print_exception(const joedb::Exception &e)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::cerr << "Error: " << e.what() << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 int main_exception_catcher(int (*main)(int, char**), int argc, char **argv)
 ////////////////////////////////////////////////////////////////////////////
 {
  try
  {
   return main(argc, argv);
  }
  catch (const joedb::Exception &e)
  {
   print_exception(e);
   return 1;
  }
 }
}

#endif
