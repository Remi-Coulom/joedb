#include "joedb/journal/Posix_File.h"
#include "joedb/Exception.h"

#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sstream>

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 void Posix_File::throw_last_error
 /////////////////////////////////////////////////////////////////////////////
 (
  const char *action,
  const char *file_name
 ) const
 {
  std::stringstream message;
  message << action << ' ' << file_name << ": " << strerror(errno) << '.';
  throw Exception(message.str());
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
 Posix_File::Posix_File(const char *file_name, Open_Mode mode):
 /////////////////////////////////////////////////////////////////////////////
  Generic_File(mode)
 {
  if (mode == Open_Mode::write_existing_or_create_new)
  {
   fd = open(file_name, O_RDWR | O_CREAT | O_EXCL, 00644);

   if (fd >= 0)
    mode = Open_Mode::create_new;
   else
   {
    fd = open(file_name, O_RDWR);
    mode = Open_Mode::write_existing;
   }

   set_mode(mode);
  }
  else if (mode == Open_Mode::create_new)
   fd = open(file_name, O_RDWR | O_CREAT | O_EXCL, 00644);
  else if (mode == Open_Mode::write_existing)
   fd = open(file_name, O_RDWR);
  else
   fd = open(file_name, O_RDONLY);

  if (fd < 0)
   throw_last_error("Opening", file_name);

  if (mode != Open_Mode::read_existing && flock(fd, LOCK_EX | LOCK_NB))
  {
   close(fd);
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
   return -1;
 }

 /////////////////////////////////////////////////////////////////////////////
 Posix_File::~Posix_File()
 /////////////////////////////////////////////////////////////////////////////
 {
  try {flush();} catch (...) {}
  close(fd);
 }
}
