#include "joedb/io/Interpreter.h"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/io/File_Parser.h"
#include "joedb/interpreter/Database.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/concurrency/Interpreted_Client.h"

#include <iostream>
#include <optional>

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 static int main(int argc, char **argv)
 /////////////////////////////////////////////////////////////////////////////
 {
  const bool default_only = false;
  const bool include_shared = false;

  File_Parser file_parser
  (
   Open_Mode::write_existing_or_create_new,
   default_only,
   include_shared
  );

  if (argc <= 1)
  {
   std::cerr << "usage: " << argv[0] << " <file> [<blob_file>]\n\n";
   file_parser.print_help(std::cerr);
   return 1;
  }

  int arg_index = 1;

  std::ostream null_stream(nullptr);
  Generic_File &file = file_parser.parse(null_stream, argc, argv, arg_index);

  std::optional<File_Parser> blob_file_parser;
  Generic_File *blob_file = nullptr;

  if (arg_index < argc)
  {
   blob_file_parser.emplace();
   blob_file = &blob_file_parser->parse(null_stream, argc, argv, arg_index);
  }

  if (file.is_readonly() || (blob_file && blob_file->is_readonly()))
  {
   Database db;
   Readonly_Journal journal(file);
   journal.replay_log(db);
   Readable_Interpreter interpreter(db, blob_file ? blob_file : &file);
   interpreter.main_loop(std::cin, std::cout);
  }
  else
  {
   joedb::Connection connection;
   Interpreted_Client client(connection, file);

   std::optional<Writable_Journal> blob_journal;
   if (blob_file)
   {
    blob_journal.emplace(*blob_file);
    blob_journal->append();
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
     blob_journal->default_checkpoint();
   });
  }

  return 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_exception_catcher(joedb::main, argc, argv);
}
