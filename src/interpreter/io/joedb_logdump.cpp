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

  if (!file.is_good())
  {
   std::cerr << "error: could not open file: " << argv[1] << '\n';
   return 1;
  }

  joedb::Journal_File journal(file);
  joedb::Dump_Listener dump_listener(std::cout);
  journal.replay_log(dump_listener);
 }

 return 0;
}
