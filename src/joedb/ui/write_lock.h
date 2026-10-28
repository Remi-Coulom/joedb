#ifndef joedb_write_lock_declared
#define joedb_write_lock_declared

#include "joedb/journal/File.h"
#include "joedb/ui/Arguments.h"

#include <iostream>

namespace joedb
{
 inline int lock(Arguments &arguments)
 {
  const beman::cstring_view file_name = arguments.get_next("file_name");

  if (arguments.missing())
  {
   arguments.print_help(std::cerr);
   return 1;
  }

  std::cout << "Locking " << file_name << "...";
  std::cout.flush();

  File lock(file_name.c_str(), Open_Mode::write_lock);

  std::cout << "\nLocked. Enter to stop.";
  std::cout.flush();
  std::cin.get();

  return 0;
 }
}

#endif