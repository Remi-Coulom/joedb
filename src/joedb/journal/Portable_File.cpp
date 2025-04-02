#include "joedb/journal/Portable_File.h"
#include "joedb/error/Exception.h"

namespace joedb::detail
{
 /////////////////////////////////////////////////////////////////////////////
 Portable_File_Buffer::Portable_File_Buffer
 /////////////////////////////////////////////////////////////////////////////
 (
  const char * const file_name,
  const Open_Mode mode
 )
 {
  constexpr auto in = std::ios::binary | std::ios::in;

  if (mode == Open_Mode::read_existing)
  {
   filebuf.open(file_name, in);
  }
  else if (mode == Open_Mode::write_existing)
  {
   filebuf.open(file_name, in | std::ios::out);
  }
  else if (mode == Open_Mode::create_new)
  {
   if (filebuf.open(file_name, in))
    throw Exception("File already exists: " + std::string(file_name));
   filebuf.open(file_name, in | std::ios::out | std::ios::trunc);
  }
  else if (mode == Open_Mode::write_existing_or_create_new)
  {
   filebuf.open(file_name, in | std::ios::out) ||
   filebuf.open(file_name, in | std::ios::out | std::ios::trunc);
  }
  else
  {
   throw Exception
   (
    std::string(file_name) + ": unsupported mode for Portable_File"
   );
  }

  if (!filebuf.is_open())
   throw Exception("Cannot open file: " + std::string(file_name));
 }
}
