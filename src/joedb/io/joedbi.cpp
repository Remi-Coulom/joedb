#include "joedb/Readable_Multiplexer.h"
#include "joedb/io/Interpreter.h"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/interpreter/Database.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/journal/Memory_File.h"

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
  std::unique_ptr<joedb::Generic_File> file;
  joedb::Memory_File *pipe = nullptr;

  const std::string file_name(argv[1]);

  if (file_name == "--pipe")
  {
   pipe = new joedb::Memory_File(joedb::Open_Mode::create_new);
   file.reset(pipe);
  }
  else
   try
   {
    file.reset
    (
     new joedb::File
     (
      file_name,
      joedb::Open_Mode::write_existing_or_create_new
     )
    );
   }
   catch (const joedb::Exception &)
   {
    file.reset
    (
     new joedb::File
     (
      file_name,
      joedb::Open_Mode::read_existing
     )
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
   {
    joedb::Writable_Journal journal(*file);
    journal.replay_log(db);
    joedb::Readable_Multiplexer multiplexer(db);
    multiplexer.add_writable(journal);
    joedb::Interpreter interpreter(multiplexer);
    interpreter.set_echo(!pipe);
    interpreter.main_loop(std::cin, std::cout);
   }

   if (pipe)
   {
    const std::vector<char> data = pipe->get_data();
    std::cout.write(data.data(), std::streamsize(data.size()));
   }
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
