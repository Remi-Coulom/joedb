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
 template<typename Main>
 static int wrap_main(Main main, int argc, char **argv)
 {
  // necessary for fast console output in Windows
  std::setvbuf(stdout, nullptr, _IOFBF, 8192);
  std::setvbuf(stderr, nullptr, _IOFBF, 8192);
  std::cerr.unsetf(std::ios_base::unitbuf);

  try
  {
   Arguments arguments(argc, argv);
   const int result = main(arguments);
   return result;
  }
  catch (const std::exception &e)
  {
   std::cerr << "Exception caught: " << e.what() << '\n';
   return 1;
  }
 }

 int main_wrapper(int (*main)(Arguments &), int argc, char **argv)
 {
  return wrap_main(main, argc, argv);
 }

 int main_wrapper
 (
  int (*main)(Arguments &, std::istream &, std::ostream &, std::ostream &),
  int argc,
  char **argv
 )
 {
  return wrap_main([main](Arguments &arguments)
  {
   return main(arguments, std::cin, std::cout, std::cerr);
  }, argc, argv);
 }

}
