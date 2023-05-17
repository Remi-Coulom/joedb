#include "joedb/journal/Portable_File.h"
#include "joedb/Exception.h"

namespace joedb
{
 static constexpr int supported_open_modes = 3;
 static const std::ios_base::openmode openmode[supported_open_modes] =
 {
  std::ios_base::binary | std::ios_base::in,
  std::ios_base::binary | std::ios_base::in | std::ios_base::out,
  std::ios_base::binary | std::ios_base::in | std::ios_base::out | std::ios_base::trunc
 };

 /////////////////////////////////////////////////////////////////////////////
 bool Portable_File_Buffer::try_open(const char *file_name, Open_Mode mode)
 /////////////////////////////////////////////////////////////////////////////
 {
  actual_mode = mode;

  const size_t index = static_cast<size_t>(mode);
  if (index >= supported_open_modes)
   throw joedb::Exception("Portable_File_Buffer: unsupported open mode");

  return filebuf.open(file_name, openmode[index]);
 }

 /////////////////////////////////////////////////////////////////////////////
 Portable_File_Buffer::Portable_File_Buffer
 /////////////////////////////////////////////////////////////////////////////
 (
  const char *file_name,
  Open_Mode mode
 )
 {
  if
  (
   mode == Open_Mode::write_existing_or_create_new ||
   mode == Open_Mode::shared_write
  )
  {
   try_open(file_name, Open_Mode::write_existing) ||
   try_open(file_name, Open_Mode::create_new);
  }
  else if (mode == Open_Mode::create_new)
  {
   if (try_open(file_name, Open_Mode::read_existing))
   {
    throw Exception("File already exists: " + std::string(file_name));
   }
   else
    try_open(file_name, Open_Mode::create_new);
  }
  else
   try_open(file_name, mode);

  if (!filebuf.is_open())
   throw Exception("Cannot open file: " + std::string(file_name));
 }
}
