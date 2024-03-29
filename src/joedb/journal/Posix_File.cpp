#include "joedb/journal/Posix_File.h"
#include "joedb/Exception.h"

#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

namespace joedb
{
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
 bool Posix_File::try_exclusive_lock()
 /////////////////////////////////////////////////////////////////////////////
 {
  return flock(fd, LOCK_EX | LOCK_NB) == 0;
 }

 /////////////////////////////////////////////////////////////////////////////
 // NOLINTNEXTLINE(readability-make-member-function-const)
 void Posix_File::shared_lock()
 /////////////////////////////////////////////////////////////////////////////
 {
  if (flock(fd, LOCK_SH) == -1)
   throw_last_error("Locking", "file");

#ifndef NDEBUG
  if (!locked)
   locked = true;
  else
   throw Exception("locking a locked file\n");
#endif
 }

 /////////////////////////////////////////////////////////////////////////////
 // NOLINTNEXTLINE(readability-make-member-function-const)
 void Posix_File::exclusive_lock()
 /////////////////////////////////////////////////////////////////////////////
 {
  if (flock(fd, LOCK_EX) == -1)
   throw_last_error("Locking", "file");

#ifndef NDEBUG
  if (!locked)
   locked = true;
  else
   throw Exception("locking a locked file\n");
#endif
 }

 /////////////////////////////////////////////////////////////////////////////
 // NOLINTNEXTLINE(readability-make-member-function-const)
 void Posix_File::unlock()
 /////////////////////////////////////////////////////////////////////////////
 {
  if (flock(fd, LOCK_UN) == -1)
   throw_last_error("Unlocking", "file");

#ifndef NDEBUG
  if (locked)
   locked = false;
  else
   throw Exception("unlocking an unlocked file\n");
#endif
 }

 /////////////////////////////////////////////////////////////////////////////
 size_t Posix_File::raw_read(char *buffer, size_t size)
 /////////////////////////////////////////////////////////////////////////////
 {
  const ssize_t result = ::read(fd, buffer, size);

  if (result < 0)
   throw_last_error("Reading", "file");

  return size_t(result);
 }

 /////////////////////////////////////////////////////////////////////////////
 void Posix_File::raw_write(const char *buffer, size_t size)
 /////////////////////////////////////////////////////////////////////////////
 {
  size_t written = 0;

  while (written < size)
  {
   const ssize_t result = ::write(fd, buffer + written, size - written);

   if (result < 0)
    throw_last_error("Writing", "file");
   else
    written += size_t(result);
  }
 }

 /////////////////////////////////////////////////////////////////////////////
 int Posix_File::raw_seek(int64_t offset)
 /////////////////////////////////////////////////////////////////////////////
 {
  return int64_t(lseek(fd, off_t(offset), SEEK_SET)) != offset;
 }

 /////////////////////////////////////////////////////////////////////////////
 void Posix_File::sync()
 /////////////////////////////////////////////////////////////////////////////
 {
  fsync(fd);
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
    exclusive_lock();
   else if (!try_exclusive_lock())
    throw_last_error("Locking", file_name);
  }
 }

 /////////////////////////////////////////////////////////////////////////////
 int64_t Posix_File::raw_get_size() const
 /////////////////////////////////////////////////////////////////////////////
 {
  struct stat s;

  if (fstat(fd, &s) == 0)
   return int64_t(s.st_size);
  else
   throw_last_error("Getting size of", "file");

  return -1;
 }

 /////////////////////////////////////////////////////////////////////////////
 Posix_File::~Posix_File()
 /////////////////////////////////////////////////////////////////////////////
 {
  if (fd >= 0)
  {
   destructor_flush();
   close(fd);
  }
 }
}
