#include <iostream>

#include "File.h"
#include "Journal_File.h"
#include "Dump_Listener.h"

int main(int argc, char **argv)
{
 if (argc <= 1)
 {
  std::cerr << "usage: " << argv[0] << " <file.joedb>\n";
  return 1;
 }
 else
 {
  joedb::File file(argv[1], joedb::File::mode_t::read_existing);

  if (file.get_status() == joedb::File::status_t::locked)
  {
   std::cerr << "error: " << argv[1] << " is locked by another process\n";
   return 1;
  }
  else if (file.get_status() != joedb::File::status_t::success)
  {
   std::cerr << "error: could not open file: " << argv[1] << '\n';
   return 1;
  }

  joedb::Journal_File journal(file);
  joedb::Dump_Listener dump_listener(std::cout);
  journal.replay_log(dump_listener);

  std::cout << "---> ";
  static const char * status_string[] =
  {
   "no_error",
   "bad_file",
   "unsupported_version",
   "bad_format",
   "crash_check"
  };
  std::cout << status_string[int(journal.get_state())] << '\n';
 }

 return 0;
}
