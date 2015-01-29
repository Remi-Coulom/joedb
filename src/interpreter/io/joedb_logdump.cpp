#include <iostream>

#include "File.h"
#include "JournalFile.h"
#include "DumpListener.h"

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

  joedb::JournalFile journal(file);
  joedb::DumpListener dump_listener(std::cout);
  journal.replay_log(dump_listener);
 }

 return 0;
}
