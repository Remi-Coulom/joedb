#ifndef joedb_Shared_Memory_File_declared
#define joedb_Shared_Memory_File_declared

#include "joedb/journal/Memory_File.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Shared_Memory_File_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   std::vector<char> &data;
   // TODO: std::mutex ???

  public:
   Shared_Memory_File_Data(std::vector<char> &data): data(data) {}
 };

 using Shared_Memory_File = Memory_File_Template<Shared_Memory_File_Data>;
}

#endif
