#ifndef joedb_Journal_Pair_declared
#define joedb_Journal_Pair_declared

#include "joedb/journal/File.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/io/main_exception_catcher.h"

#include <iostream>
#include <cstring>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 int process_journal_pair
 ////////////////////////////////////////////////////////////////////////////
 (
  int argc,
  char **argv,
  void (*process)(Readonly_Journal &, Writable_Journal &)
 )
 {
  if (argc != 3 && argc != 4)
  {
   std::cerr << "usage: " << argv[0] << " <input.joedb> <output.joedb> [--fix]\n";
   return 1;
  }

  try
  {
   const char *input_file_name = argv[1];
   const char *output_file_name = argv[2];
   const bool ignore_errors = (argc == 4 && std::strcmp(argv[3], "--fix") == 0);

   File input_file(input_file_name, Open_Mode::read_existing);
   Readonly_Journal input_journal(input_file, ignore_errors);

   File output_file(output_file_name, Open_Mode::create_new);
   Writable_Journal output_journal(output_file);

   process(input_journal, output_journal);
  }
  catch (const Exception &e)
  {
   joedb::print_exception(e);
   return 1;
  }

  return 0;
 }
}

#endif
