#ifndef joedb_joedb_to_json_declared
#define joedb_joedb_to_json_declared

#include "joedb/ui/json.h"
#include "joedb/ui/Arguments.h"
#include "joedb/interpreted/Database.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Readonly_Journal.h"

#include <istream>
#include <ostream>

namespace joedb
{
 inline int joedb_to_json
 (
  Arguments &arguments,
  std::istream &in,
  std::ostream &out,
  std::ostream &err
 )
 {
  const bool base64 = arguments.has_flag("base64");
  const beman::cstring_view file_name = arguments.get_next("file.joedb");

  if (arguments.missing())
  {
   arguments.print_help(err);
   return 1;
  }

  File file(file_name.c_str(), Open_Mode::read_existing);
  Readonly_Journal journal(file);
  Database db;
  journal.replay_log(db);
  const int error = write_json(out, db, base64);

  if (error & JSON_Error::utf8)
   err << "warning: a string could not be encoded. Maybe you should use --base64 instead.\n";

  if (error & JSON_Error::infnan)
   err << "warning: inf or nan value encoded as 0. JSON does not support inf and nan.\n";

  return 0;
 }
}

#endif
