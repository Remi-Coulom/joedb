#include "joedb/journal/Readonly_Encoded_File.h"

namespace joedb
{
 //////////////////////////////////////////////////////////////////////////
 size_t Readonly_Encoded_File::pread
 //////////////////////////////////////////////////////////////////////////
 (
  char * const buffer,
  const size_t size,
  const int64_t offset
 ) const
 {
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
      read_buffer.resize(size_t(db.get_size(b)));

     decoder.decode
     (
      blob_reader.read_blob(db.get_data(b)),
      read_buffer.data(),
      size_t(db.get_size(b))
     );

     decoded_buffer = b;
    }

    std::memcpy
    (
     buffer + intersection_start - start,
     read_buffer.data() + intersection_start - b_start,
     intersection_size
    );
   }
  }

  return size_t(global_end - offset);
 }

 //////////////////////////////////////////////////////////////////////////
 Readonly_Encoded_File::Readonly_Encoded_File
 //////////////////////////////////////////////////////////////////////////
 (
  Decoder &decoder,
  db::encoded_file::Database &db,
  const Buffered_File &blob_reader,
  Open_Mode mode
 ):
  Buffered_File(mode),
  db(db),
  blob_reader(blob_reader),
  decoded_buffer{0},
  decoder(decoder)
 {
 }

 //////////////////////////////////////////////////////////////////////////
 Readonly_Encoded_File::Readonly_Encoded_File
 //////////////////////////////////////////////////////////////////////////
 (
  Decoder &decoder,
  db::encoded_file::Database &db,
  const Buffered_File &blob_reader
 ):
  Readonly_Encoded_File(decoder, db, blob_reader, Open_Mode::read_existing)
 {
 }

 //////////////////////////////////////////////////////////////////////////
 int64_t Readonly_Encoded_File::get_size() const
 //////////////////////////////////////////////////////////////////////////
 {
  int64_t result = 0;

  for (const auto buffer: db.get_buffer_table())
  {
   const int64_t end = db.get_offset(buffer) + db.get_size(buffer);
   if (end > result)
    result = end;
  }

  return result;
 }
}
