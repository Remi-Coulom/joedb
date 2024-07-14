#include "joedb/journal/Portable_File.h"
#include "joedb/Exception.h"

#include <array>

namespace joedb
{
 static constexpr int supported_open_modes = 3;
 static constexpr std::array<std::ios_base::openmode, supported_open_modes> openmode
 {
  std::ios_base::binary | std::ios_base::in,
  std::ios_base::binary | std::ios_base::in | std::ios_base::out,
  std::ios_base::binary | std::ios_base::in | std::ios_base::out
 };

 /////////////////////////////////////////////////////////////////////////////
 bool Portable_File_Buffer::try_open(const char *file_name, Open_Mode mode)
 /////////////////////////////////////////////////////////////////////////////
 {
  const size_t index = static_cast<size_t>(mode);

  if (index < supported_open_modes)
   return filebuf.open(file_name, openmode[index]);
  else
   throw Exception
   (
    std::string(file_name) + ": unsupported mode for Portable_File"
   );
 }

 /////////////////////////////////////////////////////////////////////////////
 Portable_File_Buffer::Portable_File_Buffer
 /////////////////////////////////////////////////////////////////////////////
 (
  const char *file_name,
  Open_Mode mode
 )
 {
  if (mode == Open_Mode::create_new)
  {
   if (try_open(file_name, Open_Mode::read_existing))
    throw Exception("File already exists: " + std::string(file_name));
   else
    try_open(file_name, Open_Mode::create_new);
  }
  else
   try_open(file_name, mode);

  if (!filebuf.is_open())
   throw Exception("Cannot open file: " + std::string(file_name));
 }
}
