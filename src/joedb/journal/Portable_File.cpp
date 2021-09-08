#include "joedb/journal/Portable_File.h"
#include "joedb/Exception.h"

namespace joedb
{
 static const std::ios_base::openmode openmode[3] =
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
  return filebuf.open(file_name, openmode[static_cast<size_t>(mode)]);
 }

 /////////////////////////////////////////////////////////////////////////////
 Portable_File_Buffer::Portable_File_Buffer
 /////////////////////////////////////////////////////////////////////////////
 (
  const char *file_name,
  Open_Mode new_mode
 )
 {
  if
  (
   new_mode == Open_Mode::write_existing_or_create_new ||
   new_mode == Open_Mode::shared_write
  )
  {
   try_open(file_name, Open_Mode::write_existing) ||
   try_open(file_name, Open_Mode::create_new);
  }
  else if (new_mode == Open_Mode::create_new)
  {
   if (try_open(file_name, Open_Mode::read_existing))
   {
    throw Exception("File already exists: " + std::string(file_name));
   }
   else
    try_open(file_name, Open_Mode::create_new);
  }
  else
   try_open(file_name, new_mode);

  if (!filebuf.is_open())
   throw Exception("Cannot open file: " + std::string(file_name));
 }
}
