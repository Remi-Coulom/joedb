#ifndef joedb_Encoded_File_declared
#define joedb_Encoded_File_declared

#include "joedb/db/encoded_file.h"
#include "joedb/journal/Codec.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Readonly_Encoded_File: public Generic_File
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   encoded_file::Database &db;
   Blob_Reader &blob_reader;

   std::vector<char> read_buffer;
   encoded_file::id_of_buffer decoded_buffer;

  protected:
   Codec &codec;

   size_t pread
   (
    char * const buffer,
    const size_t size,
    const int64_t offset
   )
   override;

   Readonly_Encoded_File
   (
    Codec &codec,
    encoded_file::Database &db,
    Blob_Reader &blob_reader,
    Open_Mode mode
   );

  public:
   Readonly_Encoded_File
   (
    Codec &codec,
    encoded_file::Database &db,
    Blob_Reader &blob_reader
   );

   int64_t get_size() const override;
 };

 ////////////////////////////////////////////////////////////////////////////
 class Encoded_File: public Readonly_Encoded_File
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   encoded_file::Generic_File_Database &db;

   static constexpr size_t write_buffer_total_size = 1 << 20;
   std::vector<char> write_buffer;
   int64_t write_buffer_offset;
   int64_t write_buffer_size;

   void write_blob(const char *buffer, size_t size, size_t offset);
   void flush_write_buffer();

  protected:
   void raw_sync() override;

   size_t pread
   (
    char * const buffer,
    const size_t size,
    const int64_t offset
   )
   override;

   void pwrite(const char *buffer, size_t size, int64_t offset) override;

  public:
   Encoded_File
   (
    Codec &codec,
    encoded_file::Generic_File_Database &db
   );

   int64_t get_size() const override;

   ~Encoded_File();
 };
}

#endif
