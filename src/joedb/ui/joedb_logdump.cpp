#include "joedb/Selective_Writable.h"
#include "joedb/Multiplexer.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Readonly_Journal.h"
#include "joedb/journal/diagnostics.h"
#include "joedb/ui/Interpreter_Dump_Writable.h"
#include "joedb/ui/SQL_Dump_Writable.h"
#include "joedb/ui/Raw_Dump_Writable.h"
#include "joedb/ui/main_exception_catcher.h"
#include "joedb/interpreted/Database.h"

#include <iostream>
#include <memory>
#include <cstring>
#include <optional>

namespace joedb::ui
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
 static int logdump_main(int argc, char **argv)
 /////////////////////////////////////////////////////////////////////////////
 {
  if (argc <= 1)
  {
   std::cerr << "usage: " << argv[0];
   std::cerr << " [--sql] [--sqlite] [--raw] [--header] [--schema-only] [--ignore-errors] [--load] [--print-checkpoint] [--blob] <file.joedb>\n";
   return 1;
  }
  else
  {
   int arg_index = 1;

 #define OPTION(b, s)\
  bool b = false;\
  if (arg_index < argc && std::strcmp(argv[arg_index], s) == 0)\
  {\
   b = true;\
   arg_index++;\
  }

   OPTION(sql, "--sql");
   OPTION(sqlite, "--sqlite");
   OPTION(raw, "--raw");
   OPTION(header, "--header");
   OPTION(schema_only, "--schema-only");
   OPTION(ignore_errors, "--ignore-errors");
   OPTION(load, "--load");
   OPTION(print_checkpoint, "--print-checkpoint");
   OPTION(blob, "--blob");

   if (arg_index != argc - 1)
    return logdump_main(1, argv);

   File file(argv[arg_index], Open_Mode::read_existing);

   if (header)
   {
    dump_header(std::cout, file);
    std::cout << '\n';
    about_joedb(std::cout);
   }
   else
   {
    std::optional<Readonly_Journal> journal;

    try
    {
     journal.emplace
     (
      file,
      ignore_errors ?
       Readonly_Journal::Check::none :
       Readonly_Journal::Check::all
     );
    }
    catch (const Exception &e)
    {
     if (!ignore_errors)
     {
      std::cout << "Error opening journal file: " << e.what() << '\n';
      std::cout << "run with the --ignore-errors flag to skip this check.\n";
     }
     return 1;
    }

    std::unique_ptr<Writable> writable;

    if (sql)
     writable.reset(new SQL_Dump_Writable(std::cout));
    else if (sqlite)
     writable.reset(new SQL_Dump_Writable(std::cout, false));
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
     Multiplexer multiplexer{db, *writable};
     dump(*journal, multiplexer, print_checkpoint);
    }
    else
     dump(*journal, *writable, print_checkpoint);
   }
  }

  return 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::ui::main_exception_catcher(joedb::ui::logdump_main, argc, argv);
}
