#include "tutorial/Writable_Database.h"
#include "tutorial/File_Client.h"
#include "tutorial/Readable.h"
#include "tutorial/Multiplexer.h"
#include "joedb/ui/main_wrapper.h"
#include "joedb/ui/Command_Interpreter.h"
#include "joedb/ui/Blob_Reader_Command_Processor.h"
#include "joedb/ui/Data_Manipulation_Command_Processor.h"
#include "joedb/ui/Writable_Command_Processor.h"

#include <iostream>
#include <joedb/Multiplexer.h>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 static int tutorial_interpreter(Arguments &arguments)
 ////////////////////////////////////////////////////////////////////////////
 {
  const std::string_view file_name = arguments.get_next("file.joedb");

  if (arguments.missing())
  {
   arguments.print_help(std::cerr);
   return 1;
  }

  tutorial::File_Client client(file_name.data());

  client.transaction([](tutorial::Writable_Database &db){
   tutorial::Readable readable(db);
   tutorial::Multiplexer multiplexer(db);

   Command_Interpreter interpreter;

   Blob_Reader_Command_Processor blob_reader_processor
   (
    db.get_journal().get_file()
   );

   Data_Manipulation_Command_Processor data_manipulation_processor
   (
    readable,
    multiplexer,
    Record_Id::null
   );

   Writable_Command_Processor writable_processor(multiplexer);

   interpreter.add_processor(blob_reader_processor);
   interpreter.add_processor(data_manipulation_processor);
   interpreter.add_processor(writable_processor);

   interpreter.main_loop(std::cin, std::cout);
  });

  return 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::main_wrapper(joedb::tutorial_interpreter, argc, argv);
}
