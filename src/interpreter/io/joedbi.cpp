#include "Interpreter.h"
#include "Database.h"
#include "File.h"
#include "Journal_File.h"
#include "Readable_Multiplexer.h"

#include <iostream>
#include <memory>

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 try
 {
  joedb::Database db;

  if (argc <= 1)
  {
   joedb::Interpreter interpreter(db);
   interpreter.main_loop(std::cin, std::cout);
  }
  else
  {
   std::shared_ptr<joedb::File> file;

   try
   {
    file = std::make_shared<joedb::File>
    (
     argv[1],
     joedb::Open_Mode::write_existing_or_create_new
    );
   }
   catch (const joedb::Exception &)
   {
    file = std::make_shared<joedb::File>
    (
     argv[1],
     joedb::Open_Mode::read_existing
    );
   }

   if (file->get_mode() == joedb::Open_Mode::read_existing)
   {
    joedb::Readonly_Journal journal(*file);
    journal.replay_log(db);
    joedb::Readonly_Interpreter interpreter(db);
    interpreter.main_loop(std::cin, std::cout);
   }
   else
   {
    joedb::Journal_File journal(*file);
    journal.replay_log(db);
    joedb::Readable_Multiplexer multiplexer(db);
    multiplexer.add_writeable(journal);
    joedb::Interpreter interpreter(multiplexer);
    interpreter.main_loop(std::cin, std::cout);
   }
  }
 }
 catch (const joedb::Exception &e)
 {
  std::cerr << "Error: " << e.what() << '\n';
  return 1;
 }

 return 0;
}
