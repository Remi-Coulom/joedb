#include "Interpreter.h"
#include "Database.h"
#include "File.h"
#include "Journal_File.h"
#include "DB_Listener.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 if (argc <= 1)
 {
  joedb::Database db;
  joedb::Interpreter interpreter(db);
  interpreter.main_loop(std::cin, std::cout);
 }
 else
 {
  joedb::File file;

  file.open(argv[1], joedb::File::mode_t::write_existing);
  if (file.get_status() != joedb::File::status_t::success)
  {
   std::cout << "Could not open " << argv[1] << " for writing.\n";

   file.open(argv[1], joedb::File::mode_t::read_existing);
   if (file.get_status() != joedb::File::status_t::success)
   {
    std::cout << "Could not open " << argv[1] << " for reading.\n";

    if (file.get_status() == joedb::File::status_t::locked)
    {
     std::cout << "Error: " << argv[1] << " is locked by another process.\n";
     return 1;
    }

    file.open(argv[1], joedb::File::mode_t::create_new);
    if (file.get_status() != joedb::File::status_t::success)
    {
     std::cout << "Could not create " << argv[1] << ".\n";
     return 1;
    }
    else
     std::cout << "Created new database: " << argv[1] << '\n';
   }
   else
    std::cout << "Database opened read-only: " << argv[1] << '\n';
  }
  else
   std::cout << "Database opened successfully: " << argv[1] << '\n';

  joedb::Journal_File journal(file);
  joedb::Database db;
  joedb::DB_Listener db_listener(db);
  journal.replay_log(db_listener);

  if (journal.get_state() != joedb::Journal_File::state_t::no_error ||
      !db_listener.is_good())
  {
   std::cout << "Error reading database\n";
   std::cout << "db_listener.is_good() = " << db_listener.is_good() << '\n';
   return 1;
  }

  db.set_listener(journal);
  joedb::Interpreter interpreter(db);
  interpreter.main_loop(std::cin, std::cout);
 }

 return 0;
}
