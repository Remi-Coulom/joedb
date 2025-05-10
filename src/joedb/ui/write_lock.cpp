#include "joedb/journal/File.h"
#include "joedb/ui/main_wrapper.h"
#include "joedb/ui/Arguments.h"

#include <iostream>

namespace joedb
{
 /////////////////////////////////////////////////////////////////
 static int lock(Arguments &arguments)
 /////////////////////////////////////////////////////////////////
 {
  const std::string_view file_name = arguments.get_next("file_name");

  if (arguments.missing())
  {
   arguments.print_help(std::cerr);
   return 1;
  }

  std::cout << "Locking " << file_name << "...";
  std::cout.flush();

  File lock(file_name.data(), Open_Mode::write_lock);

  std::cout << "\nLocked. Enter to stop.";
  std::cout.flush();
  std::cin.get();

  return 0;
 }
}

//////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
//////////////////////////////////////////////////////////////////
{
 joedb::main_wrapper(joedb::lock, argc, argv);
}
