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
   static constexpr size_t big_buffer_total_size = 1 << 20;
   std::vector<char> big_buffer;

   int64_t big_buffer_offset;
   int64_t big_buffer_write_size;
   encoded_file::id_of_buffer decoded_buffer;

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
    flush_write_buffer();

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

      if (b != decoded_buffer)
      {
       codec.decode
       (
        db.read_blob_data(db.get_data(b)),
        big_buffer.data(),
        big_buffer.size()
       );

       decoded_buffer = b;
      }

      std::copy_n
      (
       big_buffer.data() + intersection_start - b_start,
       intersection_size,
       buffer + intersection_start - start
      );
     }
    }

    return total_size;
   }

   //////////////////////////////////////////////////////////////////////////
   void write_blob(const char *buffer, size_t size, size_t offset)
   //////////////////////////////////////////////////////////////////////////
   {
    const Blob blob = db.write_blob_data(codec.encode(buffer, size));
    db.new_buffer(blob, size, offset);
    db.checkpoint();
   }

   //////////////////////////////////////////////////////////////////////////
   void flush_write_buffer()
   //////////////////////////////////////////////////////////////////////////
   {
    if (big_buffer_write_size > 0)
    {
     write_blob(big_buffer.data(), big_buffer_write_size, big_buffer_offset);
     big_buffer_write_size = 0;
    }
   }

   //////////////////////////////////////////////////////////////////////////
   void pwrite(const char *buffer, size_t size, int64_t offset) override
   //////////////////////////////////////////////////////////////////////////
   {
    if (size > size_t(big_buffer_total_size))
    {
     flush_write_buffer();
     write_blob(buffer, size, offset);
    }

    if
    (
     big_buffer_write_size &&
     (
      big_buffer_offset + big_buffer_write_size != offset ||
      big_buffer_write_size + size > big_buffer_total_size
     )
    )
    {
     flush_write_buffer();
    }

    if (big_buffer_write_size == 0)
     big_buffer_offset = offset;

    std::copy_n(buffer, size, big_buffer.data() + big_buffer_write_size);

    big_buffer_write_size += size;
    decoded_buffer = encoded_file::id_of_buffer{0};
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
    db(db),
    big_buffer(big_buffer_total_size),
    big_buffer_offset(0),
    big_buffer_write_size(0),
    decoded_buffer{0}
   {
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t get_size() const override
   //////////////////////////////////////////////////////////////////////////
   {
    int64_t result = big_buffer_offset + big_buffer_write_size;

    for (const auto buffer: db.get_buffer_table())
    {
     const int64_t size = db.get_offset(buffer) + db.get_size(buffer);
     if (size > result)
      result = size;
    }

    return result;
   }

   //////////////////////////////////////////////////////////////////////////
   ~Encoded_File()
   //////////////////////////////////////////////////////////////////////////
   {
    try
    {
     flush_write_buffer();
    }
    catch (...)
    {
    }
   }
 };
}

#endif
