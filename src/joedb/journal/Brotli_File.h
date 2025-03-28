#ifndef joedb_Brotli_File_declared
#define joedb_Brotli_File_declared

#ifdef JOEDB_HAS_BROTLI

#include "joedb/journal/Brotli_Codec.h"
#include "joedb/journal/Encoded_File.h"
#include "joedb/db/encoded_file/File_Database.h"
#include "joedb/db/encoded_file/Readonly_Database.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Brotli_File_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   Brotli_Codec codec;
   db::encoded_file::File_Database db;

  public:
   Brotli_File_Data(const char *file_name);
 };

 ////////////////////////////////////////////////////////////////////////////
 class Brotli_File: private Brotli_File_Data, public Encoded_File
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   Brotli_File(const char *file_name);
 };

 ////////////////////////////////////////////////////////////////////////////
 class Readonly_Brotli_File_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   Brotli_Codec codec;
   File file;
   db::encoded_file::Readonly_Database db;

  public:
   Readonly_Brotli_File_Data(const char *file_name);
 };

 /// \ingroup journal
 class Readonly_Brotli_File:
  private Readonly_Brotli_File_Data,
  public Readonly_Encoded_File
 {
  public:
   Readonly_Brotli_File(const char *file_name);
 };
}

#endif

#endif
