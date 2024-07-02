#ifndef joedb_Encoded_File_declared
#define joedb_Encoded_File_declared

#include "joedb/db/encoded_file.h"
#include "joedb/journal/Codec.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Encoded_File: public Generic_File
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Codec &codec;
   encoded_file::Generic_File_Database &db;

  protected:
   //////////////////////////////////////////////////////////////////////////
   size_t pread
   //////////////////////////////////////////////////////////////////////////
   (
    char * const buffer,
    const size_t size,
    const int64_t offset
   )
   override
   {
    const int64_t start = offset;
    const int64_t end = offset + size;

    size_t total_size = 0;

    for (auto b: db.get_buffer_table())
    {
     const int64_t b_start = db.get_offset(b);
     const int64_t b_end = b_start + db.get_size(b);

     const int64_t intersection_start = std::max(start, b_start);
     const int64_t intersection_end = std::min(end, b_end);
     const int64_t intersection_size = intersection_end - intersection_start;

     if (intersection_size > 0)
     {
      total_size += size_t(intersection_size);

      std::string decoded(b_end - b_start, 0);

      codec.decode
      (
       db.read_blob_data(db.get_data(b)),
       decoded.data(),
       decoded.size()
      );

      std::copy_n
      (
       decoded.data() + intersection_start - b_start,
       intersection_size,
       buffer + intersection_start - start
      );
     }
    }

    return total_size;
   }

   //////////////////////////////////////////////////////////////////////////
   void pwrite(const char *buffer, size_t size, int64_t offset) override
   //////////////////////////////////////////////////////////////////////////
   {
    Blob blob = db.write_blob_data(codec.encode(buffer, size));
    db.new_buffer(blob, size, offset);
   }

  public:
   //////////////////////////////////////////////////////////////////////////
   Encoded_File
   //////////////////////////////////////////////////////////////////////////
   (
    Codec &codec,
    encoded_file::Generic_File_Database &db
   ):
    Generic_File(Open_Mode::write_existing_or_create_new),
    codec(codec),
    db(db)
   {
   }
 };
}

#endif
