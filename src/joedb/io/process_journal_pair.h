#ifndef joedb_process_journal_pair_declared
#define joedb_process_journal_pair_declared

#include "joedb/journal/File.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/io/main_exception_catcher.h"

#include <iostream>
#include <cstring>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 inline int process_journal_pair
 ////////////////////////////////////////////////////////////////////////////
 (
  int argc,
  char **argv,
  void (*process)(Readonly_Journal &, Writable_Journal &)
 )
 {
  if (argc != 3 && argc != 4)
  {
   std::cerr << "usage: " << argv[0];
   std::cerr << " [--ignore-errors] <input.joedb> <output.joedb> \n";
   return 1;
  }

  bool ignore_errors = false;
  int arg_index = 1;

  if (arg_index + 2 < argc && std::string(argv[arg_index]) == "--ignore-errors")
  {
   ignore_errors = true;
   arg_index++;
  }

  const char *input_file_name = argv[arg_index];
  const char *output_file_name = argv[arg_index + 1];

  File input_file(input_file_name, Open_Mode::read_existing);
  Readonly_Journal input_journal(input_file, ignore_errors);

  File output_file(output_file_name, Open_Mode::create_new);
  Writable_Journal output_journal(output_file);

  process(input_journal, output_journal);

  return 0;
 }
}

#endif
