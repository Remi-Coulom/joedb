#include <iostream>

#include "joedb/File.h"
#include "joedb/Journal_File.h"
#include "Interpreter_Dump_Writeable.h"
#include "SQL_Dump_Writeable.h"
#include "file_error_message.h"
#include "diagnostics.h"

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 if (argc <= 1)
 {
  std::cerr << "usage: " << argv[0] << " [--sql] [--header|--force] <file.joedb>\n";
  return 1;
 }
 else
 {
  bool sql = false;
  bool header = false;
  bool force = false;
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
   if (sql)
   {
    joedb::SQL_Dump_Writeable dump_writeable(std::cout);
    journal.replay_log(dump_writeable);
   }
   else
   {
    joedb::Interpreter_Dump_Writeable dump_writeable(std::cout);
    journal.replay_log(dump_writeable);
   }
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
  std::cerr << status_string[int(journal.get_state())] << '\n';
 }

 return 0;
}
