#ifndef joedb_Readonly_Brotli_File_declared
#define joedb_Readonly_Brotli_File_declared

#include "joedb/journal/Brotli_Decoder.h"
#include "joedb/journal/Readonly_Encoded_File.h"
#include "joedb/db/encoded_file/Readonly_Database.h"

namespace joedb
{
 namespace detail
 {
  class Readonly_Brotli_File_Data
  {
   protected:
    Brotli_Decoder decoder;
    File file;
    db::encoded_file::Readonly_Database db;

   public:
    Readonly_Brotli_File_Data(const char *file_name);
  };
 }

 /// @ingroup journal
 class Readonly_Brotli_File:
  private detail::Readonly_Brotli_File_Data,
  public Readonly_Encoded_File
 {
  public:
   Readonly_Brotli_File(const char *file_name);
 };
}

#endif
