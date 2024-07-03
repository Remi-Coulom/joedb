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

   std::vector<char> read_buffer;
   encoded_file::id_of_buffer decoded_buffer;

   static constexpr size_t write_buffer_total_size = 1 << 20;
   std::vector<char> write_buffer;
   int64_t write_buffer_offset;
   int64_t write_buffer_size;

  protected:
   //////////////////////////////////////////////////////////////////////////
   void raw_sync() override
   //////////////////////////////////////////////////////////////////////////
   {
    flush_write_buffer();
    db.checkpoint_full_commit();
   }

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

    //
    // Loop over already written buffers
    //
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
       if (int64_t(read_buffer.size()) < db.get_size(b))
        read_buffer.resize(db.get_size(b));

       codec.decode
       (
        db.read_blob_data(db.get_data(b)),
        read_buffer.data(),
        db.get_size(b)
       );

       decoded_buffer = b;
      }

      std::copy_n
      (
       read_buffer.data() + intersection_start - b_start,
       intersection_size,
       buffer + intersection_start - start
      );
     }
    }

    //
    // Read from the write buffer
    //
    {
     const int64_t b_start = write_buffer_offset;
     const int64_t b_end = b_start + write_buffer_size;

     const int64_t intersection_start = std::max(start, b_start);
     const int64_t intersection_end = std::min(end, b_end);
     const int64_t intersection_size = intersection_end - intersection_start;

     if (intersection_size > 0)
     {
      total_size += size_t(intersection_size);
      std::copy_n
      (
       write_buffer.data() + intersection_start - b_start,
       intersection_size,
       buffer + intersection_start - start
      );
     }
    }

    return total_size; // this is buggy if multiple writes to same area
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
    if (write_buffer_size > 0)
    {
     write_blob(write_buffer.data(), write_buffer_size, write_buffer_offset);
     write_buffer_size = 0;
    }
   }

   //////////////////////////////////////////////////////////////////////////
   void pwrite(const char *buffer, size_t size, int64_t offset) override
   //////////////////////////////////////////////////////////////////////////
   {
    if (size > write_buffer_total_size)
    {
     flush_write_buffer();
     write_blob(buffer, size, offset);
     return;
    }

    if
    (
     write_buffer_size &&
     (
      write_buffer_offset + write_buffer_size != offset ||
      write_buffer_size + size > write_buffer_total_size
     )
    )
    {
     flush_write_buffer();
    }

    if (write_buffer_size == 0)
     write_buffer_offset = offset;

    std::copy_n(buffer, size, write_buffer.data() + write_buffer_size);

    write_buffer_size += size;
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
    decoded_buffer{0},
    write_buffer(write_buffer_total_size),
    write_buffer_offset(0),
    write_buffer_size(0)
   {
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t get_size() const override
   //////////////////////////////////////////////////////////////////////////
   {
    int64_t result = write_buffer_offset + write_buffer_size;

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
