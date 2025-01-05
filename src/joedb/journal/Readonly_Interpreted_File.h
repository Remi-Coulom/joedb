#ifndef joedb_Readonly_Interpreted_File_declared
#define joedb_Readonly_Interpreted_File_declared

#include "joedb/journal/Memory_File.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/journal/fstream.h"
#include "joedb/interpreter/Database.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Readonly_Interpreted_File: public Memory_File
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   Writable_Journal journal;
   Database db;

   Readonly_Interpreted_File(std::istream &stream, bool readonly);

  public:
   Readonly_Interpreted_File(std::istream &stream):
    Readonly_Interpreted_File(stream, true)
   {
   }

   Readonly_Interpreted_File(std::istream &&stream):
    Readonly_Interpreted_File(stream)
   {
   }

   Readonly_Interpreted_File(const char *file_name):
    Readonly_Interpreted_File(joedb::ifstream(file_name))
   {
   }

   ~Readonly_Interpreted_File();
 };
}

#endif
