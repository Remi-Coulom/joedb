#include "joedb/journal/Brotli_File.h"

namespace joedb
{
 detail::Brotli_File_Data::Brotli_File_Data(const char *file_name): db(file_name)
 {
 }

 Brotli_File::Brotli_File(const char *file_name):
  detail::Brotli_File_Data(file_name),
  Encoded_File(Brotli_File_Data::codec, Brotli_File_Data::db)
 {
 }
}
