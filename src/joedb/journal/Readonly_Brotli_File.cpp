#include "joedb/journal/Readonly_Brotli_File.h"

namespace joedb
{
 detail::Readonly_Brotli_File_Data::Readonly_Brotli_File_Data(const char *file_name):
  file(file_name, Open_Mode::read_existing),
  db(file)
 {
 }

 Readonly_Brotli_File::Readonly_Brotli_File(const char *file_name):
  detail::Readonly_Brotli_File_Data(file_name),
  Readonly_Encoded_File
  (
   Readonly_Brotli_File_Data::decoder,
   Readonly_Brotli_File_Data::db,
   file
  )
 {
 }
}
