#include "joedb/Readable_Multiplexer.h"
#include "joedb/io/Interpreter.h"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/interpreter/Database.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Writable_Journal.h"

#include <iostream>
#include <memory>

/////////////////////////////////////////////////////////////////////////////
int joedbi_main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
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
   joedb::Writable_Journal journal(*file);
   journal.replay_log(db);
   joedb::Readable_Multiplexer multiplexer(db);
   multiplexer.add_writable(journal);
   joedb::Interpreter interpreter(multiplexer);
   interpreter.main_loop(std::cin, std::cout);
  }
 }

 return 0;
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_exception_catcher(joedbi_main, argc, argv);
}
