#include <iostream>
#include <memory>

#include "File.h"
#include "Readonly_Journal.h"
#include "Interpreter_Dump_Writeable.h"
#include "SQL_Dump_Writeable.h"
#include "diagnostics.h"
#include "Selective_Writeable.h"
#include "Raw_Dump_Writeable.h"
#include "main_exception_catcher.h"

/////////////////////////////////////////////////////////////////////////////
int joedb_logdump_main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 if (argc <= 1)
 {
  std::cerr << "usage: " << argv[0];
  std::cerr << " [--sql] [--raw] [--header] [--schema-only] [--ignore-errors] <file.joedb>\n";
  return 1;
 }
 else
 {
  bool sql = false;
  bool raw = false;
  bool header = false;
  bool schema_only = false;
  bool ignore_errors = false;

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

   std::shared_ptr<joedb::Writeable> writeable;

   if (sql)
    writeable = std::make_shared<joedb::SQL_Dump_Writeable>(std::cout);
   else if (raw)
    writeable = std::make_shared<joedb::Raw_Dump_Writeable>(std::cout);
   else
    writeable = std::make_shared<joedb::Interpreter_Dump_Writeable>(std::cout);

   if (schema_only)
   {
    joedb::Selective_Writeable w(*writeable, joedb::Selective_Writeable::Mode::schema);
    journal->replay_log(w);
   }
   else
    journal->replay_log(*writeable);
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
