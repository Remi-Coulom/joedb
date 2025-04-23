#include "joedb/ui/process_journal_pair.h"
#include "joedb/journal/File.h"

#include <iostream>
#include <sstream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 int process_journal_pair
 ////////////////////////////////////////////////////////////////////////////
 (
  int argc,
  char **argv,
  void (*process)(Readonly_Journal &, Writable_Journal &, int64_t checkpoint)
 )
 {
  int arg_index = 1;

  bool ignore_errors = false;
  if (arg_index + 2 < argc && std::string(argv[arg_index]) == "--ignore-errors")
  {
   ignore_errors = true;
   arg_index++;
  }

  int64_t until = 0;
  if (arg_index + 3 < argc && std::string(argv[arg_index]) == "--until")
  {
   std::istringstream(argv[arg_index + 1]) >> until;
   arg_index += 2;
  }

  if (arg_index + 2 != argc)
  {
   std::cerr << "usage: " << argv[0];
   std::cerr << " [--ignore-errors] [--until <checkpoint>] <input.joedb> <output.joedb> \n";
   return 1;
  }

  const char *input_file_name = argv[arg_index];
  const char *output_file_name = argv[arg_index + 1];

  File input_file(input_file_name, Open_Mode::read_existing);

  Readonly_Journal input_journal
  (
   Journal_Construction_Lock(input_file, ignore_errors)
  );

  File output_file(output_file_name, Open_Mode::create_new);
  Writable_Journal output_journal(output_file);

  process(input_journal, output_journal, until);

  return 0;
 }
}
