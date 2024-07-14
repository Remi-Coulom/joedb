#include "joedb/journal/Portable_File.h"
#include "joedb/Exception.h"

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 void Portable_File_Buffer::ensure_created(const char *file_name)
 /////////////////////////////////////////////////////////////////////////////
 {
  filebuf.open
  (
   file_name,
   std::ios::binary | std::ios::in | std::ios::out | std::ios::app
  );

  filebuf.close();
 }

 /////////////////////////////////////////////////////////////////////////////
 Portable_File_Buffer::Portable_File_Buffer
 /////////////////////////////////////////////////////////////////////////////
 (
  const char * const file_name,
  const Open_Mode mode
 )
 {
  if (mode == Open_Mode::read_existing)
  {
   filebuf.open(file_name, std::ios::binary | std::ios::in);
  }
  else if (mode == Open_Mode::write_existing)
  {
   filebuf.open(file_name, std::ios::binary | std::ios::in | std::ios::out);
  }
  else if (mode == Open_Mode::create_new)
  {
   if (filebuf.open(file_name, std::ios::in))
    throw Exception("File already exists: " + std::string(file_name));
   else
   {
    ensure_created(file_name);
    filebuf.open(file_name, std::ios::binary | std::ios::in | std::ios::out);
   }
  }
  else if (mode == Open_Mode::write_existing_or_create_new)
  {
   ensure_created(file_name);
   filebuf.open(file_name, std::ios::binary | std::ios::in | std::ios::out);
  }
  else
   throw Exception
   (
    std::string(file_name) + ": unsupported mode for Portable_File"
   );

  if (!filebuf.is_open())
   throw Exception("Cannot open file: " + std::string(file_name));
 }
}
