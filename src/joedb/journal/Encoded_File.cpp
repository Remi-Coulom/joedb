#include "joedb/journal/Encoded_File.h"

namespace joedb
{
 //////////////////////////////////////////////////////////////////////////
 void Encoded_File::raw_sync()
 //////////////////////////////////////////////////////////////////////////
 {
  flush_write_buffer();
  db.checkpoint_full_commit();
 }

 //////////////////////////////////////////////////////////////////////////
 size_t Encoded_File::pread
 //////////////////////////////////////////////////////////////////////////
 (
  char * const buffer,
  const size_t size,
  const int64_t offset
 )
 {
  flush_write_buffer();

  const int64_t start = offset;
  const int64_t end = offset + int64_t(size);

  int64_t global_end = 0;

  for (auto b: db.get_buffer_table())
  {
   const int64_t b_start = db.get_offset(b);
   const int64_t b_end = b_start + db.get_size(b);

   const int64_t intersection_start = std::max(start, b_start);
   const int64_t intersection_end = std::min(end, b_end);
   const int64_t intersection_size = intersection_end - intersection_start;

   if (intersection_size > 0)
   {
    if (intersection_end > global_end)
     global_end = intersection_end;

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

  return global_end - offset;
 }

 //////////////////////////////////////////////////////////////////////////
 void Encoded_File::write_blob(const char *buffer, size_t size, size_t offset)
 //////////////////////////////////////////////////////////////////////////
 {
  const Blob blob = db.write_blob_data(codec.encode(buffer, size));
  db.new_buffer(blob, int64_t(size), int64_t(offset));
  db.checkpoint();
 }

 //////////////////////////////////////////////////////////////////////////
 void Encoded_File::flush_write_buffer()
 //////////////////////////////////////////////////////////////////////////
 {
  if (write_buffer_size > 0)
  {
   write_blob(write_buffer.data(), write_buffer_size, write_buffer_offset);
   write_buffer_size = 0;
  }
 }

 //////////////////////////////////////////////////////////////////////////
 void Encoded_File::pwrite(const char *buffer, size_t size, int64_t offset)
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

  write_buffer_size += int64_t(size);
 }

 //////////////////////////////////////////////////////////////////////////
 Encoded_File::Encoded_File
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
 int64_t Encoded_File::get_size() const
 //////////////////////////////////////////////////////////////////////////
 {
  int64_t result = write_buffer_offset + write_buffer_size;

  for (const auto buffer: db.get_buffer_table())
  {
   const int64_t end = db.get_offset(buffer) + db.get_size(buffer);
   if (end > result)
    result = end;
  }

  return result;
 }

 //////////////////////////////////////////////////////////////////////////
 Encoded_File::~Encoded_File()
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
}
