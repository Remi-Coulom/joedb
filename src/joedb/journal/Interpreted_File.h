#ifndef joedb_Interpreted_File_declared
#define joedb_Interpreted_File_declared

#include "joedb/journal/Memory_File.h"

#include <fstream>

namespace joedb
{
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
