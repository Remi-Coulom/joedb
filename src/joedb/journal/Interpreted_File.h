#ifndef joedb_Interpreted_File_declared
#define joedb_Interpreted_File_declared

#include "joedb/journal/Readonly_Memory_File.h"
#include "joedb/journal/Memory_File.h"

#include <fstream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Interpreted_File_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   Memory_File memory_file;

  public:
   Interpreted_File_Data(std::istream &file);
 };

 ////////////////////////////////////////////////////////////////////////////
 class Interpreted_File:
 ////////////////////////////////////////////////////////////////////////////
  private Interpreted_File_Data,
  public Readonly_Memory_File
 {
  public:
   Interpreted_File(std::istream &file):
    Interpreted_File_Data(file),
    Readonly_Memory_File(memory_file.get_data())
   {
   }

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
