#include "joedb/ui/main_wrapper.h"

#include <iostream>

namespace joedb
{
 /// Process command-line arguments and catch exceptions from main
 ///
 /// This function is particularly necessary in Windows, because no
 /// exception information is printed by default there.
 int main_wrapper
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
