#include "joedb/ui/main_exception_catcher.h"

#include <iostream>

namespace joedb
{
 /// Catch exception from main
 ///
 /// This function is particularly necessary in Windows, because no
 /// exception information is printed by default there.
 int main_exception_catcher
 (
  int (*main)(Arguments &), int argc, char **argv
 )
 {
  try
  {
   Arguments arguments(argc, argv);
   return main(arguments);
  }
  catch (const std::exception &e)
  {
   std::cerr << "Exception caught: " << e.what() << '\n';
   return 1;
  }
 }
}
