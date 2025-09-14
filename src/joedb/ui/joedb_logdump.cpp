#include "joedb/Selective_Writable.h"
#include "joedb/Multiplexer.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Readonly_Journal.h"
#include "joedb/ui/diagnostics.h"
#include "joedb/ui/Interpreter_Dump_Writable.h"
#include "joedb/ui/SQL_Dump_Writable.h"
#include "joedb/ui/Raw_Dump_Writable.h"
#include "joedb/ui/main_wrapper.h"
#include "joedb/interpreted/Database.h"

#include <iostream>
#include <memory>
#include <cstring>
#include <optional>

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 static void dump
 /////////////////////////////////////////////////////////////////////////////
 (
  Readonly_Journal &journal,
  Writable &writable,
  bool print_checkpoint
 )
 {
  if (print_checkpoint)
   journal.replay_with_checkpoint_comments(writable);
  else
   journal.replay_log(writable);
 }

 /////////////////////////////////////////////////////////////////////////////
 static int logdump(Arguments &arguments)
 /////////////////////////////////////////////////////////////////////////////
 {
  const bool sql = arguments.has_flag("sql");
  const bool raw = arguments.has_flag("raw");
  const bool header = arguments.has_flag("header");
  const bool schema_only = arguments.has_flag("schema_only");
  const bool ignore_header = arguments.has_flag("ignore_header");
  const bool load = arguments.has_flag("load");
  const bool print_checkpoint = arguments.has_flag("print_checkpoint");
  const bool blob = arguments.has_flag("blob");
  const std::string_view file_name = arguments.get_next("file.joedb");

  if (arguments.missing())
  {
   arguments.print_help(std::cerr);
   return 1;
  }

  File file(file_name.data(), Open_Mode::read_existing);

  if (header)
   dump_header(std::cout, file);
  else
  {
   std::optional<Readonly_Journal> journal;

   journal.emplace
   (
    Journal_Construction_Lock
    (
     file,
     ignore_header ? Recovery::ignore_header : Recovery::none
    )
   );

   std::unique_ptr<Writable> writable;

   const Abstract_File *blob_reader = blob ? &file : nullptr;

   if (sql)
    writable.reset(new SQL_Dump_Writable(std::cout, blob_reader));
   else if (raw)
    writable.reset(new Raw_Dump_Writable(std::cout));
   else
    writable.reset(new Interpreter_Dump_Writable(std::cout, blob));

   if (schema_only)
   {
    Selective_Writable selective_writable
    (
     *writable,
     Selective_Writable::Mode::schema
    );
    journal->replay_log(selective_writable);
   }
   else if (load)
   {
    Database db;
    Multiplexer multiplexer{*writable, db};
    dump(*journal, multiplexer, print_checkpoint);
   }
   else
    dump(*journal, *writable, print_checkpoint);
  }

  return 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_wrapper(joedb::logdump, argc, argv);
}
