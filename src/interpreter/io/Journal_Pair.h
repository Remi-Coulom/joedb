#ifndef joedb_Journal_Pair_declared
#define joedb_Journal_Pair_declared

#include "File.h"
#include "Journal_File.h"

#include <iostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 int process_journal_pair
 ////////////////////////////////////////////////////////////////////////////
 (
  int argc,
  char **argv,
  void (*process)(Readonly_Journal &, Journal_File &)
 )
 {
  if (argc != 3)
  { 
   std::cerr << "usage: " << argv[0] << " <input.joedb> <output.joedb>\n";
   return 1;
  } 
 
  try
  {
   File input_file(argv[1], Open_Mode::read_existing);
   Readonly_Journal input_journal(input_file);

   File output_file(argv[2], Open_Mode::create_new);
   Journal_File output_journal(output_file);

   process(input_journal, output_journal);
  }
  catch (const Exception &e)
  {
   std::cerr << "Error: " << e.what() << '\n';
   return 1;
  }
 
  return 0;
 }
}

#endif
