#ifndef joedb_Journal_Pair_declared
#define joedb_Journal_Pair_declared

#include "File.h"
#include "Journal_File.h"

#include <iostream>

namespace joedb
{
 int process_journal_pair
 (
  int argc,
  char **argv,
  void (*process)(Journal_File &, Journal_File &)
 )
 {
  if (argc != 3)
  { 
   std::cerr << "usage: " << argv[0] << " <input.joedb> <output.joedb>\n";
   return 1;
  } 
 
  File input_file(argv[1], File::mode_t::read_existing);
  Journal_File input_journal(input_file);
  if (input_journal.get_state() != joedb::Journal_File::state_t::no_error)
  { 
   std::cerr << "error: could not open input file: " << argv[1] << '\n';
   return 1;
  } 
 
  {
   File test_existing(argv[2], File::mode_t::read_existing);
   if (test_existing.get_status() == joedb::File::status_t::success)
   {
    std::cerr << "error: " << argv[2] << " already exists\n";
    return 1;
   }
  }
 
  File output_file(argv[2], File::mode_t::create_new);
  Journal_File output_journal(output_file);
  if (output_journal.get_state() != joedb::Journal_File::state_t::no_error)
  {
   std::cerr << "error: could not open output file: " << argv[2] << '\n';
   return 1;
  }

  process(input_journal, output_journal);
 
  return 0;
 }
}

#endif
