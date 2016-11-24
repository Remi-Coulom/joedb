#include "json.h"
#include "File.h"
#include "Journal_File.h"
#include "file_error_message.h"
#include "DB_Listener.h"
#include "Database.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 if (argc <= 1)
 {
  std::cerr << "usage: " << argv[0] << "<file.joedb>\n";
  return 1;
 }

 joedb::File file(argv[1], joedb::File::mode_t::read_existing);

 if (joedb::file_error_message(std::cerr, file))
  return 1;

 joedb::Journal_File journal(file);
 joedb::Database db;
 joedb::DB_Listener db_listener(db);
 journal.replay_log(db_listener);

 if (!journal.is_good() || !db_listener.is_good())
 {
  std::cerr << "Error reading database\n";
  return 1;
 }

 joedb::write_json(std::cout, db);

 return 0;
}
