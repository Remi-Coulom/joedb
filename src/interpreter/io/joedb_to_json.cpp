#include "json.h"
#include "File.h"
#include "Readonly_Journal.h"
#include "Database.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
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

 joedb::File file(argv[file_index], joedb::Open_Mode::read_existing);
 joedb::Readonly_Journal journal(file);
 joedb::Database db;
 journal.replay_log(db);
 const bool ok = joedb::write_json(std::cout, db, base64);

 if (!ok)
  std::cerr << "warning: a string could not be encoded. Maybe you should use --base64 instead.\n";

 return 0;
}
