#include "dump.h"
#include "File.h"
#include "Journal_File.h"

#include <iostream>

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 if (argc != 3)
 {
  std::cerr << "usage: " << argv[0] << " <input.joedb> <output.joedb>\n";
  return 1;
 }

 File input_file(argv[1], File::mode_t::read_existing);
 Journal_File input_journal(input_file);

 File output_file(argv[2], File::mode_t::create_new);
 Journal_File output_journal(output_file);

 pack(input_journal, output_journal);

 return 0;
}
