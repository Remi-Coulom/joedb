#include "joedb/journal/Windows_File.h"
#include "joedb/Exception.h"

#include <algorithm>
#include <cctype>

namespace joedb
{
 const std::array<DWORD, Windows_Handle::modes> Windows_Handle::desired_access
 {
  GENERIC_READ,
  GENERIC_READ | GENERIC_WRITE,
  GENERIC_READ | GENERIC_WRITE,
  GENERIC_READ | GENERIC_WRITE,
  GENERIC_READ | GENERIC_WRITE,
  GENERIC_READ | GENERIC_WRITE
 };

 const std::array<DWORD, Windows_Handle::modes> Windows_Handle::share_mode
 {
  FILE_SHARE_READ | FILE_SHARE_WRITE,
  FILE_SHARE_READ,
  FILE_SHARE_READ,
  FILE_SHARE_READ,
  FILE_SHARE_READ | FILE_SHARE_WRITE,
  FILE_SHARE_READ | FILE_SHARE_WRITE
 };

 const std::array<DWORD, Windows_Handle::modes> Windows_Handle::creation_disposition
 {
  OPEN_EXISTING,
  OPEN_EXISTING,
  CREATE_NEW,
  OPEN_ALWAYS,
  OPEN_ALWAYS,
  OPEN_ALWAYS
 };

 /////////////////////////////////////////////////////////////////////////////
 static DWORD size_to_dword(size_t size)
 /////////////////////////////////////////////////////////////////////////////
 {
  return DWORD(std::min(size, size_t(1ULL << 31)));
 }

 /////////////////////////////////////////////////////////////////////////////
 void Windows_Handle::throw_last_error
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
 BOOL Windows_Handle::lock(Lock_Operation op, int64_t start, int64_t size)
 /////////////////////////////////////////////////////////////////////////////
 {
  if (start < 0 || size < 0)
   return FALSE;

  ULARGE_INTEGER large_start;
  ULARGE_INTEGER large_size;

  large_start.QuadPart = uint64_t(start);
  large_size.QuadPart = size > 0 ? uint64_t(size) : uint64_t(-1);

  OVERLAPPED overlapped{};
  overlapped.Offset = large_start.u.LowPart;
  overlapped.OffsetHigh = large_start.u.HighPart;

  switch(op)
  {
   case Lock_Operation::shared_lock:
    return LockFileEx
    (
     file,
     0,
     0,
     large_size.u.LowPart,
     large_size.u.HighPart,
     &overlapped
    );
   break;

   case Lock_Operation::exclusive_lock:
    return LockFileEx
    (
     file,
     LOCKFILE_EXCLUSIVE_LOCK,
     0,
     large_size.u.LowPart,
     large_size.u.HighPart,
     &overlapped
    );
   break;

   case Lock_Operation::unlock:
    return UnlockFileEx
    (
     file,
     0,
     large_size.u.LowPart,
     large_size.u.HighPart,
     &overlapped
    );
   break;
  }

  return FALSE;
 }

 /////////////////////////////////////////////////////////////////////////////
 void Windows_Handle::shared_lock(int64_t start, int64_t size)
 /////////////////////////////////////////////////////////////////////////////
 {
  if (!lock(Lock_Operation::shared_lock, start, size))
   throw_last_error("Read-locking", "file");
 }

 /////////////////////////////////////////////////////////////////////////////
 void Windows_Handle::exclusive_lock(int64_t start, int64_t size)
 /////////////////////////////////////////////////////////////////////////////
 {
  if (!lock(Lock_Operation::exclusive_lock, start, size))
   throw_last_error("Write-locking", "file");
 }

 /////////////////////////////////////////////////////////////////////////////
 void Windows_Handle::unlock(int64_t start, int64_t size)
 /////////////////////////////////////////////////////////////////////////////
 {
  if (!lock(Lock_Operation::unlock, start, size))
   throw_last_error("Unlocking", "file");
 }

 /////////////////////////////////////////////////////////////////////////////
 size_t Windows_Handle::pread(char* buffer, size_t size, int64_t offset)
 /////////////////////////////////////////////////////////////////////////////
 {
  OVERLAPPED overlapped{};
  overlapped.Pointer = PVOID(offset);

  DWORD result;

  if (ReadFile(file, buffer, size_to_dword(size), &result, &overlapped))
   return size_t(result);
  else
  {
   if (GetLastError() == ERROR_HANDLE_EOF)
    return 0;
   else
    throw_last_error("Reading", "file");
  }

  return 0;
 }

 /////////////////////////////////////////////////////////////////////////////
 void Windows_Handle::pwrite
 /////////////////////////////////////////////////////////////////////////////
 (
  const char* buffer,
  size_t size,
  int64_t offset
 )
 {
  size_t written = 0;

  while (written < size)
  {
   OVERLAPPED overlapped{};
   overlapped.Pointer = PVOID(offset + written);

   DWORD actually_written;

   if
   (
    !WriteFile
    (
     file,
     buffer + written,
     size_to_dword(size - written),
     &actually_written,
     &overlapped
    )
   )
   {
    throw_last_error("Writing", "file");
   }

   written += actually_written;
  }
 }

 /////////////////////////////////////////////////////////////////////////////
 void Windows_Handle::sync()
 /////////////////////////////////////////////////////////////////////////////
 {
  if (FlushFileBuffers(file) == 0)
   throw_last_error("syncing", "file");
 }

 /////////////////////////////////////////////////////////////////////////////
 Windows_Handle::Windows_Handle(const char *file_name, const Open_Mode mode):
 /////////////////////////////////////////////////////////////////////////////
  Buffered_File(mode),
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
 }

 /////////////////////////////////////////////////////////////////////////////
 Windows_File::Windows_File(const char *file_name, const Open_Mode mode):
 /////////////////////////////////////////////////////////////////////////////
  Windows_Handle(file_name, mode)
 {
  if (mode == Open_Mode::write_lock)
   exclusive_lock_tail();
 }

 /////////////////////////////////////////////////////////////////////////////
 int64_t Windows_Handle::get_size() const
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
 Windows_Handle::~Windows_Handle()
 /////////////////////////////////////////////////////////////////////////////
 {
  destructor_flush();
  try
  {
   if (!CloseHandle(file))
    throw_last_error("Closing", "file");
  }
  catch(...)
  {
   postpone_exception("Error closing file");
  }
 }
}
