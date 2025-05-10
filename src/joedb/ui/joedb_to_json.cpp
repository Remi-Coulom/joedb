#include "joedb/ui/json.h"
#include "joedb/ui/main_exception_catcher.h"
#include "joedb/interpreted/Database.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Readonly_Journal.h"

#include <iostream>

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 static int joedb_to_json(Arguments &arguments)
 /////////////////////////////////////////////////////////////////////////////
 {
  const bool base64 = arguments.has_option("base64");
  const std::string_view file_name = arguments.get_next("file.joedb");

  if (arguments.has_missing())
  {
   arguments.print_help(std::cerr);
   return 1;
  }

  File file(file_name.data(), Open_Mode::read_existing);
  Readonly_Journal journal(file);
  Database db;
  journal.replay_log(db);
  const int error = write_json(std::cout, db, base64);

  if (error & JSON_Error::utf8)
   std::cerr << "warning: a string could not be encoded. Maybe you should use --base64 instead.\n";

  if (error & JSON_Error::infnan)
   std::cerr << "warning: inf or nan value encoded as 0. JSON does not support inf and nan.\n";

  return 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_exception_catcher(joedb::joedb_to_json, argc, argv);
}
