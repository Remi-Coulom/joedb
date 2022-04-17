#ifndef joedb_Interpreted_File_declared
#define joedb_Interpreted_File_declared

#include "joedb/journal/Memory_File.h"

#include <fstream>

namespace joedb
{
 class Generic_File;

 ////////////////////////////////////////////////////////////////////////////
 void append_interpreted_commands
 ////////////////////////////////////////////////////////////////////////////
 (
  joedb::Generic_File &joedb_file,
  std::istream &command_stream
 );

 ////////////////////////////////////////////////////////////////////////////
 class Interpreted_File: public Memory_File
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   Interpreted_File(std::istream &file);

   Interpreted_File(std::istream &&file):
    Interpreted_File(file)
   {
   }

   Interpreted_File(const char *file_name):
    Interpreted_File(std::ifstream(file_name))
   {
   }
 };
}

#endif
