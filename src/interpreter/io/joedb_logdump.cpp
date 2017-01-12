#include <iostream>
#include <memory>

#include "File.h"
#include "Readonly_Journal.h"
#include "Interpreter_Dump_Writeable.h"
#include "SQL_Dump_Writeable.h"
#include "diagnostics.h"
#include "Selective_Writeable.h"

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 if (argc <= 1)
 {
  std::cerr << "usage: " << argv[0];
  std::cerr << " [--sql] [--header] [--schema-only] <file.joedb>\n";
  return 1;
 }
 else
 {
  bool sql = false;
  bool header = false;
  bool schema_only = false;
  int arg_index = 1;

  if (arg_index + 1 < argc && std::string(argv[arg_index]) == "--sql")
  {
   sql = true;
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

  joedb::File file(argv[arg_index], joedb::Open_Mode::read_existing);

  if (header)
  {
   joedb::dump_header(std::cout, file);
   std::cout << '\n';
   joedb::about_joedb(std::cout);
  }
  else
  {
   joedb::Readonly_Journal journal(file);

   std::shared_ptr<joedb::Writeable> writeable;

   if (sql)
    writeable = std::make_shared<joedb::SQL_Dump_Writeable>(std::cout);
   else
    writeable = std::make_shared<joedb::Interpreter_Dump_Writeable>(std::cout);

   if (schema_only)
   {
    joedb::Selective_Writeable w(*writeable, joedb::Selective_Writeable::Mode::schema);
    journal.replay_log(w);
   }
   else
    journal.replay_log(*writeable);
  }
 }

 return 0;
}
