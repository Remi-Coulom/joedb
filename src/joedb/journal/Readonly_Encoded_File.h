#ifndef joedb_Readonly_Encoded_File_declared
#define joedb_Readonly_Encoded_File_declared

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
}

#endif
