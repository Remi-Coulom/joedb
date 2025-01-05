#ifndef joedb_Interpreted_File_declared
#define joedb_Interpreted_File_declared

#include "joedb/journal/Readonly_Interpreted_File.h"
#include "joedb/journal/fstream.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Interpreted_Stream_File: public Readonly_Interpreted_File
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   joedb::iostream &stream;

   void pull();
   void pwrite(const char *buffer, size_t size, int64_t offset) override;
   void shared_lock(int64_t start, int64_t size) override;
   void exclusive_lock(int64_t start, int64_t size) override;
   void unlock(int64_t start, int64_t size) override;

  public:
   Interpreted_Stream_File(joedb::iostream &stream);
 };

 ////////////////////////////////////////////////////////////////////////////
 class Interpreted_File_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   joedb::fstream file_stream;

  public:
   Interpreted_File_Data(const char *file_name, Open_Mode mode);
   ~Interpreted_File_Data();
 };

 ////////////////////////////////////////////////////////////////////////////
 class Interpreted_File:
 ////////////////////////////////////////////////////////////////////////////
  private Interpreted_File_Data,
  public Interpreted_Stream_File
 {
  public:
   Interpreted_File
   (
    const char *file_name,
    Open_Mode mode = Open_Mode::write_existing_or_create_new
   );
 };
}

#endif
