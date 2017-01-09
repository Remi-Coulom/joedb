#include "json.h"
#include "File.h"
#include "Journal_File.h"
#include "Database.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 if (argc <= 1)
 {
  std::cerr << "usage: " << argv[0] << "[--base64] <file.joedb>\n";
  return 1;
 }

 bool base64 = false;
 int file_index = 1;

 if (argc > 2 && argv[1] == std::string("--base64"))
 {
  file_index++;
  base64 = true;
 }

 joedb::File file(argv[file_index], joedb::File::mode_t::read_existing);
 joedb::Journal_File journal(file);
 joedb::Database db;
 journal.replay_log(db);
 joedb::write_json(std::cout, db, base64);

 return 0;
}
