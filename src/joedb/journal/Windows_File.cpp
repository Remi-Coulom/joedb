#include "joedb/journal/Windows_File.h"
#include "joedb/Exception.h"

namespace joedb
{
 const DWORD Windows_File::desired_access[] =
 {
  GENERIC_READ,
  GENERIC_READ | GENERIC_WRITE,
  GENERIC_READ | GENERIC_WRITE,
  GENERIC_READ | GENERIC_WRITE
 };

 const DWORD Windows_File::creation_disposition[] =
 {
  OPEN_EXISTING,
  OPEN_EXISTING,
  CREATE_NEW,
  OPEN_ALWAYS
 };

 /////////////////////////////////////////////////////////////////////////////
 void Windows_File::throw_last_error() const
 /////////////////////////////////////////////////////////////////////////////
 {
  const DWORD last_error = GetLastError();
  LPVOID buffer;

  FormatMessage
  (
   FORMAT_MESSAGE_ALLOCATE_BUFFER |
   FORMAT_MESSAGE_FROM_SYSTEM,
   NULL,
   last_error,
   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
   (LPTSTR) &buffer,
   0,
   NULL
  );

  std::string s((char *)buffer);

  LocalFree(buffer);

  throw Exception(s);
 }

 /////////////////////////////////////////////////////////////////////////////
 size_t Windows_File::read_buffer()
 /////////////////////////////////////////////////////////////////////////////
 {
  DWORD result;

  if (ReadFile(file, buffer, DWORD(buffer_size), &result, NULL))
   return size_t(result);
  else
   throw_last_error();
 }

 /////////////////////////////////////////////////////////////////////////////
 void Windows_File::write_buffer()
 /////////////////////////////////////////////////////////////////////////////
 {
  if (!WriteFile(file, buffer, DWORD(write_buffer_index), NULL, NULL))
   throw_last_error();
 }

 /////////////////////////////////////////////////////////////////////////////
 int Windows_File::seek(int64_t offset)
 /////////////////////////////////////////////////////////////////////////////
 {
  LARGE_INTEGER large_integer;
  large_integer.QuadPart = offset;

  if (SetFilePointerEx(file, large_integer, NULL, FILE_BEGIN))
   return 0;
  else
   return 1;
 }

 /////////////////////////////////////////////////////////////////////////////
 void Windows_File::sync()
 /////////////////////////////////////////////////////////////////////////////
 {
 }

 /////////////////////////////////////////////////////////////////////////////
 Windows_File::Windows_File(const char *file_name, const Open_Mode mode):
 /////////////////////////////////////////////////////////////////////////////
  file
  (
   CreateFileA
   (
    file_name,
    desired_access[static_cast<mode_type>(mode)],
    FILE_SHARE_READ,
    NULL,
    creation_disposition[static_cast<mode_type>(mode)],
    FILE_ATTRIBUTE_NORMAL,
    NULL
   )
  )
 {
  if (file == INVALID_HANDLE_VALUE)
   throw_last_error();

  this->mode = mode;

  if (mode == Open_Mode::write_existing_or_create_new && GetLastError() == 0)
   this->mode = Open_Mode::create_new;
 }

 /////////////////////////////////////////////////////////////////////////////
 int64_t Windows_File::get_size() const
 /////////////////////////////////////////////////////////////////////////////
 {
  LARGE_INTEGER result;

  if (GetFileSizeEx(file, &result))
   return int64_t(result.QuadPart);
  else
   throw_last_error();
 }

 /////////////////////////////////////////////////////////////////////////////
 Windows_File::~Windows_File()
 /////////////////////////////////////////////////////////////////////////////
 {
  flush();
  CloseHandle(file);
 }
}
