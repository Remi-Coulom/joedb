#ifndef joedb_Interpreted_File_declared
#define joedb_Interpreted_File_declared

#include "joedb/journal/Memory_File.h"

namespace joedb
{
 //////////////////////////////////////////////////////////////////////////// 
 class Interpreted_File: public Memory_File
 //////////////////////////////////////////////////////////////////////////// 
 {
  public:
   Interpreted_File(std::istream &file);
 };
}

#endif
