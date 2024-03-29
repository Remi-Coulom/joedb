#include "joedb/io/main_exception_catcher.h"
#include "joedb/Exception.h"

#include <iostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 static void print_exception(const std::exception &e)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::cerr << "Error: " << e.what() << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 int main_exception_catcher
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
