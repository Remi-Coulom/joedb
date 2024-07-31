#ifndef joedb_Interpreted_File_declared
#define joedb_Interpreted_File_declared

#include "joedb/journal/Memory_File.h"
#include "joedb/interpreter/Database.h"

#include <fstream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Readonly_Interpreted_File: public Memory_File
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   Database db;
   int64_t current_checkpoint;

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
    Readonly_Interpreted_File(std::ifstream(file_name))
   {
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Interpreted_Stream: public Readonly_Interpreted_File
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   std::iostream &stream;

   void pull();

   void pwrite(const char *buffer, size_t size, int64_t offset) override
   {
    Memory_File::pwrite(buffer, size, offset);
    if (offset == 17 || offset == 33) // second copy of checkpoint position
     pull();
   }

  public:
   Interpreted_Stream(std::iostream &stream):
    Readonly_Interpreted_File(stream, false),
    stream(stream)
   {
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Interpreted_File_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   std::fstream file_stream;

  public:
   Interpreted_File_Data(const char *file_name)
   {
    constexpr auto in = std::ios::binary | std::ios::in;
    file_stream.open(file_name, in | std::ios::out);
    if (!file_stream)
     file_stream.open(file_name, in | std::ios::out | std::ios::trunc);
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Interpreted_File:
 ////////////////////////////////////////////////////////////////////////////
  private Interpreted_File_Data,
  public Interpreted_Stream
 {
  public:
   Interpreted_File(const char *file_name):
    Interpreted_File_Data(file_name),
    Interpreted_Stream(file_stream)
   {
   }
 };
}

#endif
