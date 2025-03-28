#include "joedb/ui/json.h"
#include "joedb/ui/main_exception_catcher.h"
#include "joedb/interpreted/Database.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Readonly_Journal.h"

#include <iostream>

namespace joedb::ui
{
 /////////////////////////////////////////////////////////////////////////////
 static int joedb_to_json(int argc, char **argv)
 /////////////////////////////////////////////////////////////////////////////
 {
  if (argc <= 1)
  {
   std::cerr << "usage: " << argv[0] << " [--base64] <file.joedb>\n";
   return 1;
  }

  bool base64 = false;
  int file_index = 1;

  if (argc > 2 && argv[1] == std::string("--base64"))
  {
   file_index++;
   base64 = true;
  }

  File file(argv[file_index], Open_Mode::read_existing);
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
 return joedb::ui::main_exception_catcher(joedb::ui::joedb_to_json, argc, argv);
}
