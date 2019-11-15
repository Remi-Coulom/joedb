#ifndef joedb_main_exception_catcher_declared
#define joedb_main_exception_catcher_declared

#include "joedb/Exception.h"

#include <iostream>

namespace joedb
{
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
   std::cerr << "joedb error: " << e.what() << '\n';
   return 1;
  }
 }
}

#endif
