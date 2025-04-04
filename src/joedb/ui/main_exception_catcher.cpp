#include "joedb/ui/main_exception_catcher.h"

#include <iostream>

namespace joedb
{
 /// Catch exception from main
 ///
 /// This function is particularly necessary in Windows, becase no
 /// exception information is printed by default there.
 int main_exception_catcher
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
   std::cerr << "Exception caught: " << e.what() << '\n';
   return 1;
  }
 }
}
