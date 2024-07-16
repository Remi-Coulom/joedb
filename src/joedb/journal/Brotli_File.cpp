#include "joedb/journal/Brotli_File.h"

namespace joedb
{
 Brotli_File_Data::Brotli_File_Data(const char *file_name): db(file_name)
 {
 }

 Brotli_File::Brotli_File(const char *file_name):
  Brotli_File_Data(file_name),
  Encoded_File(Brotli_File_Data::codec, Brotli_File_Data::db)
 {
 }

 Readonly_Brotli_File_Data::Readonly_Brotli_File_Data(const char *file_name):
  file(file_name, Open_Mode::read_existing),
  db(file)
 {
 }

 Readonly_Brotli_File::Readonly_Brotli_File(const char *file_name):
  Readonly_Brotli_File_Data(file_name),
  Readonly_Encoded_File
  (
   Readonly_Brotli_File_Data::codec,
   Readonly_Brotli_File_Data::db,
   file
  )
 {
 }
}
