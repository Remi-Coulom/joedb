#include "Database.h"
#include "File.h"
#include "JournalFile.h"
#include "SchemaListener.h"

#include <iostream>

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
 if (argc <= 1)
 {
  std::cerr << "Usage: " << argv[0] << " <file.joedb>\n";
  return 1;
 }

 File file(argv[1], File::mode_t::read_existing);
 if (!file.is_good())
 {
  std::cerr << "Error: could not open " << argv[1] << '\n';
  return 1;
 }

 JournalFile journal(file);
 Database db;
 SchemaListener schema_listener(db);
 journal.replay_log(schema_listener);

 if (journal.get_state() != JournalFile::state_t::no_error ||
     schema_listener.get_error())
 {
  std::cerr << "Error reading database\n";
  return 1;
 }

 return 0;
}
