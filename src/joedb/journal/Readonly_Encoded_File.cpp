#include "joedb/journal/Readonly_Encoded_File.h"
#include <joedb/error/Exception.h>

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

  int64_t actual_end = start;

  for (auto b: db.get_buffer_table())
  {
   const int64_t b_start = db.get_offset(b);
   const int64_t b_size = db.get_size(b);

   if (b_size <= 0)
    continue;

   const int64_t b_end = int64_t(uint64_t(b_start) + uint64_t(b_size));

   const int64_t intersection_start = std::max(start, b_start);
   const int64_t intersection_end = std::min(end, b_end);

   if (intersection_start < intersection_end && intersection_start <= actual_end)
   {
    if (b != decoded_buffer)
    {
     const size_t read_buffer_size = size_t(db.get_size(b));

     if (read_buffer_size > max_buffer_size)
      continue;

     if (read_buffer.size() < read_buffer_size)
      read_buffer.resize(read_buffer_size);

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
     size_t(intersection_end - intersection_start)
    );

    if (intersection_end > actual_end)
     actual_end = intersection_end;
   }
  }

  return size_t(actual_end - start);
 }

 //////////////////////////////////////////////////////////////////////////
 Readonly_Encoded_File::Readonly_Encoded_File
 //////////////////////////////////////////////////////////////////////////
 (
  Decoder &decoder,
  const db::encoded_file::Database &db,
  const Abstract_File &blob_reader,
  Open_Mode mode
 ):
  Abstract_File(mode),
  db(db),
  blob_reader(blob_reader),
  decoded_buffer{Record_Id::null},
  decoder(decoder)
 {
 }

 //////////////////////////////////////////////////////////////////////////
 Readonly_Encoded_File::Readonly_Encoded_File
 //////////////////////////////////////////////////////////////////////////
 (
  Decoder &decoder,
  const db::encoded_file::Database &db,
  const Abstract_File &blob_reader
 ):
  Readonly_Encoded_File(decoder, db, blob_reader, Open_Mode::read_existing)
 {
 }

 //////////////////////////////////////////////////////////////////////////
 int64_t Readonly_Encoded_File::get_size() const
 //////////////////////////////////////////////////////////////////////////
 {
  uint64_t result = 0;

  for (const auto buffer: db.get_buffer_table())
  {
   const uint64_t end = uint64_t(db.get_offset(buffer)) + uint64_t(db.get_size(buffer));
   if (end > result)
    result = end;
  }

  return std::max<int64_t>(0, int64_t(result));
 }
}
