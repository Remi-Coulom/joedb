#ifndef joedb_Encoded_File_declared
#define joedb_Encoded_File_declared

#include "joedb/journal/Readonly_Encoded_File.h"
#include "joedb/db/encoded_file/Generic_File_Database.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Encoded_File: public Readonly_Encoded_File
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   db::encoded_file::Generic_File_Database &db;

   static constexpr size_t write_buffer_total_size = 1 << 20;
   std::vector<char> write_buffer;
   int64_t write_buffer_offset;
   size_t write_buffer_size;

   void write_blob(const char *buffer, size_t size, int64_t offset);
   void flush_write_buffer();

  public:
   Encoded_File
   (
    Codec &codec,
    db::encoded_file::Generic_File_Database &db
   );

   size_t pread(char *buffer, size_t size, int64_t offset) override;
   void pwrite(const char *buffer, size_t size, int64_t offset) override;
   int64_t get_size() const override;
   void sync() override;

   ~Encoded_File();
 };
}

#endif
