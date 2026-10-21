#include "joedb/ui/main_wrapper.h"

#include <iostream>
#include <cstdio>

namespace joedb
{
 /// Process command-line arguments and catch exceptions from main
 ///
 /// This function is particularly necessary in Windows, because no
 /// exception information is printed when main throws.
 ///
 /// @ingroup ui
 int main_wrapper
 (
  int (*main)(Arguments &), int argc, char **argv
 )
 {
  // necessary for fast console output in Windows
  std::setvbuf(stdout, nullptr, _IOFBF, 8192);

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
