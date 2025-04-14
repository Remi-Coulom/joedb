#include "joedb/journal/Posix_File.h"
#include "joedb/error/Exception.h"

#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#ifdef JOEDB_HAS_BROKEN_POSIX_LOCKING
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
 void Posix_FD::throw_last_error
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
 int Posix_FD::lock(int command, short type, int64_t start, int64_t size)
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
 bool Posix_FD::try_exclusive_lock(int64_t start, int64_t size)
 /////////////////////////////////////////////////////////////////////////////
 {
  return lock(JOEDB_SETLK, F_WRLCK, start, size) == 0;
 }

 /////////////////////////////////////////////////////////////////////////////
 void Posix_FD::shared_lock(int64_t start, int64_t size)
 /////////////////////////////////////////////////////////////////////////////
 {
  if (lock(JOEDB_SETLKW, F_RDLCK, start, size) < 0)
   throw_last_error("Read-locking", "file");
 }

 /////////////////////////////////////////////////////////////////////////////
 void Posix_FD::exclusive_lock(int64_t start, int64_t size)
 /////////////////////////////////////////////////////////////////////////////
 {
  if (lock(JOEDB_SETLKW, F_WRLCK, start, size) < 0)
   throw_last_error("Write-locking", "file");
 }

 /////////////////////////////////////////////////////////////////////////////
 void Posix_FD::unlock(int64_t start, int64_t size) noexcept
 /////////////////////////////////////////////////////////////////////////////
 {
  lock(JOEDB_SETLK, F_UNLCK, start, size);
 }

 /////////////////////////////////////////////////////////////////////////////
 size_t Posix_FD::pread(char *buffer, size_t size, int64_t offset) const
 /////////////////////////////////////////////////////////////////////////////
 {
  const ssize_t result = ::pread(fd, buffer, size, offset);

  if (result < 0)
   throw_last_error("Reading", "file");

  return size_t(result);
 }

 /////////////////////////////////////////////////////////////////////////////
 void Posix_FD::pwrite(const char *buffer, size_t size, int64_t offset)
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
 void Posix_FD::sync()
 /////////////////////////////////////////////////////////////////////////////
 {
#ifdef __APPLE__
  if (fcntl(fd, F_FULLFSYNC) == -1)
#else
  if (fsync(fd) == -1)
#endif
  {
   throw_last_error("syncing", "file");
  }
 }

 /////////////////////////////////////////////////////////////////////////////
 Posix_FD::Posix_FD(const char *file_name, const Open_Mode mode):
 /////////////////////////////////////////////////////////////////////////////
  Buffered_File(mode)
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
   if (fd < 0)
    fd = open(file_name, O_RDWR);
  }

  if (fd < 0)
   throw_last_error("Opening", file_name);
 }

 /////////////////////////////////////////////////////////////////////////////
 Posix_File::Posix_File(const char *file_name, const Open_Mode mode):
 /////////////////////////////////////////////////////////////////////////////
  Posix_FD(file_name, mode)
 {
  if (mode != Open_Mode::read_existing && mode != Open_Mode::shared_write)
  {
   if (mode == Open_Mode::write_lock)
    exclusive_lock_tail();
   else if (!try_exclusive_lock(last_position, 1))
    throw_last_error("Locking", file_name);
  }
 }

 /////////////////////////////////////////////////////////////////////////////
 int64_t Posix_FD::get_size() const
 /////////////////////////////////////////////////////////////////////////////
 {
  struct stat s;

  if (fstat(fd, &s) < 0)
   throw_last_error("Getting size of", "file");

  return int64_t(s.st_size);
 }

 /////////////////////////////////////////////////////////////////////////////
 Posix_FD::~Posix_FD()
 /////////////////////////////////////////////////////////////////////////////
 {
  destructor_flush();
  close(fd);
 }
}
