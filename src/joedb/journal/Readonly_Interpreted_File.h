#ifndef joedb_Readonly_Interpreted_File_declared
#define joedb_Readonly_Interpreted_File_declared

#include "joedb/journal/Memory_File.h"
#include "joedb/journal/File_View.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/journal/fstream.h"
#include "joedb/interpreted/Database.h"

namespace joedb
{
 /// Read a text file in joedbi format
 ///
 /// @ingroup journal
 class Readonly_Interpreted_File: public Memory_File
 {
  protected:
   Database db;
   File_View file_view;
   Writable_Journal journal;

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

   Readonly_Interpreted_File(const std::string &file_name):
    Readonly_Interpreted_File(file_name.data())
   {
   }

   ~Readonly_Interpreted_File();
 };
}

#endif
