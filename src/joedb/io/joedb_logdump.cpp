#include "joedb/Selective_Writable.h"
#include "joedb/Multiplexer.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Readonly_Journal.h"
#include "joedb/journal/diagnostics.h"
#include "joedb/io/Interpreter_Dump_Writable.h"
#include "joedb/io/SQL_Dump_Writable.h"
#include "joedb/io/Raw_Dump_Writable.h"
#include "joedb/io/main_exception_catcher.h"

#include <iostream>
#include <memory>

/////////////////////////////////////////////////////////////////////////////
int joedb_logdump_main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 if (argc <= 1)
 {
  std::cerr << "usage: " << argv[0];
  std::cerr << " [--sql] [--raw] [--header] [--schema-only] [--ignore-errors] [--load] <file.joedb>\n";
  return 1;
 }
 else
 {
  bool sql = false;
  bool raw = false;
  bool header = false;
  bool schema_only = false;
  bool ignore_errors = false;
  bool load = false;

  int arg_index = 1;

  if (arg_index + 1 < argc && std::string(argv[arg_index]) == "--sql")
  {
   sql = true;
   arg_index++;
  }

  if (arg_index + 1 < argc && std::string(argv[arg_index]) == "--raw")
  {
   raw = true;
   arg_index++;
  }

  if (arg_index + 1 < argc && std::string(argv[arg_index]) == "--header")
  {
   header = true;
   arg_index++;
  }

  if (arg_index + 1 < argc && std::string(argv[arg_index]) == "--schema-only")
  {
   schema_only = true;
   arg_index++;
  }

  if (arg_index + 1 < argc && std::string(argv[arg_index]) == "--ignore-errors")
  {
   ignore_errors = true;
   arg_index++;
  }

  if (arg_index + 1 < argc && std::string(argv[arg_index]) == "--load")
  {
   load = true;
   arg_index++;
  }

  joedb::File file(argv[arg_index], joedb::Open_Mode::read_existing);

  if (header)
  {
   joedb::dump_header(std::cout, file);
   std::cout << '\n';
   joedb::about_joedb(std::cout);
  }
  else
  {
   std::unique_ptr<joedb::Readonly_Journal> journal;

   try
   {
    journal.reset(new joedb::Readonly_Journal(file, ignore_errors));
   }
   catch (const joedb::Exception &e)
   {
    if (!ignore_errors)
    {
     std::cout << "Error opening journal file: " << e.what() << '\n';
     std::cout << "run with the --ignore-errors flag to skip this check.\n";
    }
    return 1;
   }

   std::unique_ptr<joedb::Writable> writable;

   if (sql)
    writable.reset(new joedb::SQL_Dump_Writable(std::cout));
   else if (raw)
    writable.reset(new joedb::Raw_Dump_Writable(std::cout));
   else
    writable.reset(new joedb::Interpreter_Dump_Writable(std::cout));

   if (schema_only)
   {
    joedb::Selective_Writable selective_writable
    (
     *writable,
     joedb::Selective_Writable::Mode::schema
    );
    journal->replay_log(selective_writable);
   }
   else if (load)
   {
    joedb::Database db;
    joedb::Multiplexer multiplexer{db, *writable};
    journal->replay_log(multiplexer);
   }
   else
    journal->replay_log(*writable);
  }
 }

 return 0;
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_exception_catcher(joedb_logdump_main, argc, argv);
}
