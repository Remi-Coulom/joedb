#include <iostream>
#include <memory>

#include "joedb/File.h"
#include "joedb/Journal_File.h"
#include "Interpreter_Dump_Writeable.h"
#include "SQL_Dump_Writeable.h"
#include "file_error_message.h"
#include "diagnostics.h"
#include "Selective_Writeable.h"

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 if (argc <= 1)
 {
  std::cerr << "usage: " << argv[0] << " [--sql] [--header|--force] [--schema-only] <file.joedb>\n";
  return 1;
 }
 else
 {
  bool sql = false;
  bool header = false;
  bool force = false;
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

  if (arg_index + 1 < argc && std::string(argv[arg_index]) == "--force")
  {
   force = true;
   arg_index++;
  }

  if (arg_index + 1 < argc && std::string(argv[arg_index]) == "--schema-only")
  {
   schema_only = true;
   arg_index++;
  }

  joedb::File file(argv[arg_index], joedb::File::mode_t::read_existing);

  if (joedb::file_error_message(std::cerr, file))
   return 1;

  joedb::Journal_File journal(file);

  if (header || journal.get_state() != joedb::Journal_File::state_t::no_error)
  {
   joedb::dump_header(std::cout, file);
   std::cout << '\n';
   joedb::about_joedb(std::cout);
   if (header)
    return 0;
  }

  if (journal.get_state() == joedb::Journal_File::state_t::no_error || force)
  {
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

  static char const * const status_string[]
  {
   "no_error",
   "bad_file",
   "unsupported_version",
   "bad_format",
   "crash_check",
   "writeable_threw"
  };

  static_assert(sizeof(status_string) / sizeof(*status_string) ==
   size_t(joedb::Journal_File::state_t::journal_errors),
   "size of status_string is wrong");

  std::cerr << "\n---> ";
  std::cerr << status_string[int(journal.get_state())];
  if (journal.get_state() == joedb::Journal_File::state_t::writeable_threw)
   std::cerr << ": " << journal.get_thrown_error() << '\n';
  else
   std::cerr << '\n';
 }

 return 0;
}
