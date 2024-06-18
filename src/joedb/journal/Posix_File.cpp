#include "joedb/journal/Posix_File.h"
#include "joedb/Exception.h"

#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#ifdef JOEDB_HAS_BRAINDEAD_POSIX_LOCKING
#define JOEDB_SETLK F_SETLK
#define JOEDB_SETLKW F_SETLKW
//#warning is C++23
//#warning Joedb is using old-style POSIX locks. File locking is broken if you open the same file more than once within the same process. New OFD locks work better and should be enabled if your system supports them.
#else
#define JOEDB_SETLK F_OFD_SETLK
#define JOEDB_SETLKW F_OFD_SETLKW
#endif

namespace joedb
{
#ifndef _FILE_OFFSET_BITS
 static_assert
 (
  sizeof(off_t) == sizeof(int64_t),
  "Define the _FILE_OFFSET_BITS macro to 32 or 64 to silence this error. 64 is recommended if possible. Joedb does not check for file-size overflow."
 );
#endif

 /////////////////////////////////////////////////////////////////////////////
 void Posix_File::throw_last_error
 /////////////////////////////////////////////////////////////////////////////
 (
  const char *action,
  const char *file_name
 )
 {
  throw Exception
  (
   std::string(action) + ' ' + file_name + ": " + strerror(errno)
  );
 }

 /////////////////////////////////////////////////////////////////////////////
 // NOLINTNEXTLINE(readability-make-member-function-const)
 int Posix_File::lock(int command, short type, int64_t start, int64_t size)
 /////////////////////////////////////////////////////////////////////////////
 {
  struct flock lock;
  lock.l_type = type;
  lock.l_whence = SEEK_SET;
  lock.l_start = off_t(start);
  lock.l_len = off_t(size);
  lock.l_pid = 0;

  return fcntl(fd, command, &lock);
 }

 /////////////////////////////////////////////////////////////////////////////
 bool Posix_File::try_exclusive_lock(int64_t start, int64_t size)
 /////////////////////////////////////////////////////////////////////////////
 {
  return lock(JOEDB_SETLK, F_WRLCK, start, size) == 0;
 }

 /////////////////////////////////////////////////////////////////////////////
 void Posix_File::shared_lock(int64_t start, int64_t size)
 /////////////////////////////////////////////////////////////////////////////
 {
  if (lock(JOEDB_SETLKW, F_RDLCK, start, size) < 0)
   throw_last_error("Read-locking", "file");
 }

 /////////////////////////////////////////////////////////////////////////////
 void Posix_File::exclusive_lock(int64_t start, int64_t size)
 /////////////////////////////////////////////////////////////////////////////
 {
  if (lock(JOEDB_SETLKW, F_WRLCK, start, size) < 0)
   throw_last_error("Write-locking", "file");
 }

 /////////////////////////////////////////////////////////////////////////////
 void Posix_File::unlock(int64_t start, int64_t size)
 /////////////////////////////////////////////////////////////////////////////
 {
  if (lock(JOEDB_SETLK, F_UNLCK, start, size) < 0)
   throw_last_error("Unlocking", "file");
 }

 /////////////////////////////////////////////////////////////////////////////
 size_t Posix_File::pread(char *buffer, size_t size, int64_t offset)
 /////////////////////////////////////////////////////////////////////////////
 {
  const ssize_t result = ::pread(fd, buffer, size, offset);

  if (result < 0)
   throw_last_error("Reading", "file");

  return size_t(result);
 }

 /////////////////////////////////////////////////////////////////////////////
 void Posix_File::pwrite(const char *buffer, size_t size, int64_t offset)
 /////////////////////////////////////////////////////////////////////////////
 {
  size_t written = 0;

  while (written < size)
  {
   const ssize_t result = ::pwrite
   (
    fd,
    buffer + written,
    size - written,
    offset + written
   );

   if (result < 0)
    throw_last_error("Writing", "file");
   else
    written += size_t(result);
  }
 }

 /////////////////////////////////////////////////////////////////////////////
 void Posix_File::raw_sync()
 /////////////////////////////////////////////////////////////////////////////
 {
  if (fsync(fd) < 0)
   throw_last_error("syncing", "file");
 }

 /////////////////////////////////////////////////////////////////////////////
 Posix_File::Posix_File(const char *file_name, const Open_Mode mode):
 /////////////////////////////////////////////////////////////////////////////
  Generic_File(mode)
 {
  if (mode == Open_Mode::read_existing)
   fd = open(file_name, O_RDONLY);
  else if (mode == Open_Mode::write_existing)
   fd = open(file_name, O_RDWR);
  else if (mode == Open_Mode::create_new)
   fd = open(file_name, O_RDWR | O_CREAT | O_EXCL, 00644);
  else if
  (
   mode == Open_Mode::write_existing_or_create_new ||
   mode == Open_Mode::shared_write ||
   mode == Open_Mode::write_lock
  )
  {
   fd = open(file_name, O_RDWR | O_CREAT | O_EXCL, 00644);

   Open_Mode new_mode;

   if (fd >= 0)
    new_mode = Open_Mode::create_new;
   else
   {
    fd = open(file_name, O_RDWR);
    new_mode = Open_Mode::write_existing;
   }

   Generic_File::set_mode(new_mode);
  }

  if (fd < 0)
   throw_last_error("Opening", file_name);

  if (mode != Open_Mode::read_existing && mode != Open_Mode::shared_write)
  {
   if (mode == Open_Mode::write_lock)
    exclusive_lock_tail();
   else if (!try_exclusive_lock(last_position, 1))
    throw_last_error("Locking", file_name);
  }
 }

 /////////////////////////////////////////////////////////////////////////////
 int64_t Posix_File::get_size() const
 /////////////////////////////////////////////////////////////////////////////
 {
  struct stat s;

  if (fstat(fd, &s) < 0)
   throw_last_error("Getting size of", "file");

  return int64_t(s.st_size);
 }

 /////////////////////////////////////////////////////////////////////////////
 Posix_File::~Posix_File()
 /////////////////////////////////////////////////////////////////////////////
 {
  destructor_flush();
  try
  {
   if (close(fd) < 0)
    throw_last_error("closing", "file");
  }
  catch (...)
  {
   postpone_exception("Error closing file");
  }
 }
}
