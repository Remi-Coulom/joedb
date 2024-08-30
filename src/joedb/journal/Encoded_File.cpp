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
  return Readonly_Encoded_File::pread(buffer, size, offset);
 }

 //////////////////////////////////////////////////////////////////////////
 void Encoded_File::write_blob(const char *buffer, size_t size, int64_t offset)
 //////////////////////////////////////////////////////////////////////////
 {
  const Blob blob = db.write_blob_data(codec.encode(buffer, size));
  db.new_buffer(blob, int64_t(size), offset);
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
    write_buffer_offset + int64_t(write_buffer_size) != offset ||
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

 //////////////////////////////////////////////////////////////////////////
 Encoded_File::Encoded_File
 //////////////////////////////////////////////////////////////////////////
 (
  Codec &codec,
  encoded_file::Generic_File_Database &db
 ):
  Readonly_Encoded_File(codec, db, db, Open_Mode::write_existing_or_create_new),
  db(db),
  write_buffer(write_buffer_total_size),
  write_buffer_offset(0),
  write_buffer_size(0)
 {
 }

 //////////////////////////////////////////////////////////////////////////
 int64_t Encoded_File::get_size() const
 //////////////////////////////////////////////////////////////////////////
 {
  return std::max
  (
   write_buffer_offset + int64_t(write_buffer_size),
   Readonly_Encoded_File::get_size()
  );
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
