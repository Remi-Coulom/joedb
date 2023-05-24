#include "joedb/journal/Windows_File.h"
#include "joedb/Exception.h"

#include <algorithm>
#include <cctype>

namespace joedb
{
 const DWORD Windows_File::desired_access[] =
 {
  GENERIC_READ,
  GENERIC_READ | GENERIC_WRITE,
  GENERIC_READ | GENERIC_WRITE,
  GENERIC_READ | GENERIC_WRITE,
  GENERIC_READ | GENERIC_WRITE,
  GENERIC_READ | GENERIC_WRITE
 };

 const DWORD Windows_File::share_mode[] =
 {
  FILE_SHARE_READ | FILE_SHARE_WRITE,
  FILE_SHARE_READ,
  FILE_SHARE_READ,
  FILE_SHARE_READ,
  FILE_SHARE_READ | FILE_SHARE_WRITE,
  FILE_SHARE_READ | FILE_SHARE_WRITE
 };

 const DWORD Windows_File::creation_disposition[] =
 {
  OPEN_EXISTING,
  OPEN_EXISTING,
  CREATE_NEW,
  OPEN_ALWAYS,
  OPEN_ALWAYS,
  OPEN_ALWAYS
 };

 /////////////////////////////////////////////////////////////////////////////
 void Windows_File::throw_last_error
 /////////////////////////////////////////////////////////////////////////////
 (
  const char *action,
  const char *file_name
 ) const
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
   (LPTSTR)&buffer,
   0,
   NULL
  );

  std::string error((const char *)buffer);
  LocalFree(buffer);
  while (!error.empty() && std::isspace(error.back()))
   error.pop_back();

  throw Exception
  (
   std::string(action) + ' ' + std::string(file_name) + ": " + error
  );
 }

 /////////////////////////////////////////////////////////////////////////////
 void Windows_File::shared_lock()
 /////////////////////////////////////////////////////////////////////////////
 {
  OVERLAPPED overlapped;
  overlapped.Offset = 0;
  overlapped.OffsetHigh = 0;
  overlapped.hEvent = 0;

  if (!LockFileEx(file, 0, 0, 1, 0, &overlapped))
  {
   throw_last_error("Locking", "file");
  }
 }

 /////////////////////////////////////////////////////////////////////////////
 void Windows_File::exclusive_lock()
 /////////////////////////////////////////////////////////////////////////////
 {
  OVERLAPPED overlapped;
  overlapped.Offset = 0;
  overlapped.OffsetHigh = 0;
  overlapped.hEvent = 0;

  if (!LockFileEx(file, LOCKFILE_EXCLUSIVE_LOCK, 0, 1, 0, &overlapped))
  {
   throw_last_error("Locking", "file");
  }
 }

 /////////////////////////////////////////////////////////////////////////////
 void Windows_File::unlock()
 /////////////////////////////////////////////////////////////////////////////
 {
  OVERLAPPED overlapped;
  overlapped.Offset = 0;
  overlapped.OffsetHigh = 0;
  overlapped.hEvent = 0;

  if (!UnlockFileEx(file, 0, 1, 0, &overlapped))
  {
   throw_last_error("Unlocking", "file");
  }
 }

 /////////////////////////////////////////////////////////////////////////////
 size_t Windows_File::raw_read(char *buffer, size_t size)
 /////////////////////////////////////////////////////////////////////////////
 {
  constexpr size_t max_size = 1ULL << 31;

  if (size > max_size)
   size = max_size;

  DWORD result;

  if (ReadFile(file, buffer, DWORD(size), &result, NULL))
   return size_t(result);
  else
   throw_last_error("Reading", "file");

  return 0;
 }

 /////////////////////////////////////////////////////////////////////////////
 void Windows_File::raw_write(const char *buffer, size_t size)
 /////////////////////////////////////////////////////////////////////////////
 {
  constexpr size_t max_size = 1ULL << 31;
  size_t written = 0;

  while (written < size)
  {
   const size_t remaining = size - written;
   const size_t block_size = std::min(max_size, remaining);

   if (!WriteFile(file, buffer + written, DWORD(block_size), NULL, NULL))
    throw_last_error("Writing", "file");

   written += block_size;
  }
 }

 /////////////////////////////////////////////////////////////////////////////
 int Windows_File::raw_seek(int64_t offset)
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
  FlushFileBuffers(file);
 }

 /////////////////////////////////////////////////////////////////////////////
 Windows_File::Windows_File(const char *file_name, const Open_Mode mode):
 /////////////////////////////////////////////////////////////////////////////
  Generic_File(mode),
  file
  (
   CreateFileA
   (
    file_name,
    desired_access[static_cast<size_t>(mode)],
    share_mode[static_cast<size_t>(mode)],
    NULL,
    creation_disposition[static_cast<size_t>(mode)],
    FILE_ATTRIBUTE_NORMAL,
    NULL
   )
  )
 {
  if (file == INVALID_HANDLE_VALUE)
   throw_last_error("Opening", file_name);

  if (creation_disposition[static_cast<size_t>(mode)] == OPEN_ALWAYS)
  {
   if (GetLastError() == 0)
    set_mode(Open_Mode::create_new);
   else
    set_mode(Open_Mode::write_existing);
  }

  if (mode == Open_Mode::write_lock)
   exclusive_lock();
 }

 /////////////////////////////////////////////////////////////////////////////
 int64_t Windows_File::raw_get_size() const
 /////////////////////////////////////////////////////////////////////////////
 {
  LARGE_INTEGER result;

  if (GetFileSizeEx(file, &result))
   return int64_t(result.QuadPart);
  else
   throw_last_error("Getting size of", "file");

  return -1;
 }

 /////////////////////////////////////////////////////////////////////////////
 Windows_File::~Windows_File()
 /////////////////////////////////////////////////////////////////////////////
 {
  destructor_flush();
  CloseHandle(file);
 }
}
