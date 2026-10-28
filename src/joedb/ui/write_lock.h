#ifndef joedb_write_lock_declared
#define joedb_write_lock_declared

#include "joedb/journal/File.h"
#include "joedb/ui/Arguments.h"

#include <istream>
#include <ostream>

namespace joedb
{
 inline int lock
 (
  Arguments &arguments,
  std::istream &in,
  std::ostream &out,
  std::ostream &err
 )
 {
  const beman::cstring_view file_name = arguments.get_next("file_name");

  if (arguments.missing())
  {
   arguments.print_help(err);
   return 1;
  }

  out << "Locking " << file_name << "...";
  out.flush();

  File lock(file_name.c_str(), Open_Mode::write_lock);

  out << "\nLocked. Enter to stop.";
  out.flush();
  in.get();

  return 0;
 }
}

#endif
