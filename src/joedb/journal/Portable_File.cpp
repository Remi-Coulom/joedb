#include "joedb/journal/Portable_File.h"
#include "joedb/Exception.h"

#include <array>

namespace joedb
{
 static constexpr int supported_open_modes = 3;
 static constexpr std::array<std::ios::openmode, supported_open_modes> openmode
 {
  std::ios::binary | std::ios::in,
  std::ios::binary | std::ios::in | std::ios::out,
  std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc
#if __cplusplus >= 202302L
  | std::ios::noreplace
#endif
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
  // Note that before C++23, the race with create_new is dangerous:
  // data written by another process after the first test may be erased.
  // There is no portable solution to this race.

  if (mode == Open_Mode::write_existing_or_create_new)
  {
   try_open(file_name, Open_Mode::write_existing) ||
   try_open(file_name, Open_Mode::create_new);
  }
  else if (mode == Open_Mode::create_new)
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
