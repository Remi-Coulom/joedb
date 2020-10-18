#ifndef joedb_Journal_Pair_declared
#define joedb_Journal_Pair_declared

#include "joedb/journal/File.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/io/main_exception_catcher.h"

#include <iostream>

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
