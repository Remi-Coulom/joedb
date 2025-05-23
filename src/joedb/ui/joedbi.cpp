#include "joedb/ui/Interpreter.h"
#include "joedb/ui/main_wrapper.h"
#include "joedb/ui/File_Parser.h"
#include "joedb/interpreted/Database.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/concurrency/Writable_Database_Client.h"

#include <iostream>
#include <optional>

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 static int joedbi(Arguments &arguments)
 /////////////////////////////////////////////////////////////////////////////
 {
  const Record_Id max_record_id
  (
   arguments.get_option<index_t>("max_record_id", "n", -1)
  );

  arguments.add_parameter("<file>");
  arguments.add_parameter("[<blob_file>]");

  const bool default_only = false;
  const bool include_shared = false;

  File_Parser file_parser
  (
   Open_Mode::write_existing_or_create_new,
   default_only,
   include_shared
  );

  std::ostream null_stream(nullptr);
  Buffered_File *file = file_parser.parse(null_stream, arguments);

  std::optional<File_Parser> blob_file_parser;
  Buffered_File *blob_file = nullptr;

  if (arguments.get_remaining_count())
  {
   blob_file_parser.emplace();
   blob_file = blob_file_parser->parse(null_stream, arguments);
  }
  else
   blob_file = file;

  if (!file)
  {
   arguments.print_help(std::cerr) << '\n';
   file_parser.print_help(std::cerr);
   return 1;
  }

  if (file->is_readonly() || (blob_file && blob_file->is_readonly()))
  {
   Database db(max_record_id);
   Readonly_Journal journal(*file);
   journal.replay_log(db);
   Readable_Interpreter interpreter(db, blob_file);
   interpreter.main_loop(std::cin, std::cout);
  }
  else
  {
   Connection connection;
   Writable_Database_Client client(*file, connection, Content_Check::fast, max_record_id);

   std::optional<Writable_Journal> blob_journal;
   if (blob_file_parser && blob_file)
   {
    blob_journal.emplace(*blob_file);
    blob_journal->skip_directly_to(blob_journal->get_checkpoint());
   }

   client.transaction([blob_file, &blob_journal]
   (
    const Readable &readable,
    Writable &writable
   )
   {
    Writable &blob_writer = blob_journal ? *blob_journal : writable;
    Interpreter interpreter(readable, writable, blob_file, blob_writer, 0);
    interpreter.main_loop(std::cin, std::cout);
    if (blob_journal)
     blob_journal->soft_checkpoint();
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
