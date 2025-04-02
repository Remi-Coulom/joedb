#ifndef joedb_Brotli_File_declared
#define joedb_Brotli_File_declared

#include "joedb/journal/Brotli_Codec.h"
#include "joedb/journal/Encoded_File.h"
#include "joedb/db/encoded_file/File_Database.h"

namespace joedb
{
 namespace detail
 {
  class Brotli_File_Data
  {
   protected:
    Brotli_Codec codec;
    db::encoded_file::File_Database db;

   public:
    Brotli_File_Data(const char *file_name);
  };
 }

 /// \ingroup journal
 class Brotli_File: private detail::Brotli_File_Data, public Encoded_File
 {
  public:
   Brotli_File(const char *file_name);
 };
}

#endif
