#ifndef joedb_Readonly_Encoded_File_declared
#define joedb_Readonly_Encoded_File_declared

#include "joedb/db/encoded_file/Database.h"
#include "joedb/journal/Buffered_File.h"
#include "joedb/journal/Decoder.h"

namespace joedb
{
 /// @ingroup journal
 class Readonly_Encoded_File: public Buffered_File
 {
  private:
   const db::encoded_file::Database &db;
   const Buffered_File &blob_reader;

   mutable std::vector<char> read_buffer;
   mutable db::encoded_file::id_of_buffer decoded_buffer;

  protected:
   Decoder &decoder;

   size_t pread(char * buffer, size_t size, int64_t offset) const override;

   Readonly_Encoded_File
   (
    Decoder &decoder,
    const db::encoded_file::Database &db,
    const Buffered_File &blob_reader,
    Open_Mode mode
   );

  public:
   Readonly_Encoded_File
   (
    Decoder &decoder,
    const db::encoded_file::Database &db,
    const Buffered_File &blob_reader
   );

   int64_t get_size() const override;
 };
}

#endif
