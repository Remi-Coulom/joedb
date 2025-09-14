#include "joedb/ui/Interpreter.h"
#include "joedb/ui/main_wrapper.h"
#include "joedb/ui/File_Parser.h"
#include "joedb/ui/Blob_Reader_Command_Processor.h"
#include "joedb/interpreted/Database.h"
#include "joedb/concurrency/Writable_Database_Client.h"

#include <iostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 static int joedbi(Arguments &arguments)
 ////////////////////////////////////////////////////////////////////////////
 {
  const Record_Id max_record_id
  (
   arguments.get_option<index_t>("max_record_id", "n", -1)
  );

  arguments.add_parameter("<file>");

  const bool default_only = false;
  const bool include_shared = false;

  File_Parser file_parser
  (
   Open_Mode::write_existing_or_create_new,
   default_only,
   include_shared
  );

  std::ostream null_stream(nullptr);
  Abstract_File *file = file_parser.parse(null_stream, arguments);

  if (!file)
  {
   arguments.print_help(std::cerr) << '\n';
   file_parser.print_help(std::cerr);
   return 1;
  }

  Blob_Reader_Command_Processor blob_processor(*file);

  if (file->is_readonly())
  {
   Database db(max_record_id);
   Readonly_Journal journal(*file);
   journal.replay_log(db);
   Readable_Interpreter interpreter(db);
   interpreter.add_processor(blob_processor);
   interpreter.main_loop(std::cin, std::cout);
  }
  else
  {
   Writable_Database_Client client
   (
    *file,
    Connection::dummy,
    Content_Check::fast,
    max_record_id
   );

   client.transaction([max_record_id, &blob_processor]
   (
    const Readable &readable,
    Writable &writable
   )
   {
    Interpreter interpreter(readable, writable, max_record_id);
    interpreter.add_processor(blob_processor);
    interpreter.main_loop(std::cin, std::cout);
   });
  }

  return 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_wrapper(joedb::joedbi, argc, argv);
}
