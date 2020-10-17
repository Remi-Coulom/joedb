#include "joedb/journal/Posix_File.h"
#include "joedb/Exception.h"

#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 bool Posix_File::lock_file()
 /////////////////////////////////////////////////////////////////////////////
 {
  return flock(fd, LOCK_EX | LOCK_NB) == 0;
 }

 /////////////////////////////////////////////////////////////////////////////
 size_t Posix_File::read_buffer()
 /////////////////////////////////////////////////////////////////////////////
 {
  const ssize_t result = ::read(fd, buffer, buffer_size);

  if (result < 0)
   throw Exception("Error reading file");

  return size_t(result);
 }

 /////////////////////////////////////////////////////////////////////////////
 void Posix_File::write_buffer()
 /////////////////////////////////////////////////////////////////////////////
 {
  const ssize_t result = ::write(fd, buffer, write_buffer_index);
  if (result != ssize_t(write_buffer_index))
   throw Exception("Error writing file");
 }

 /////////////////////////////////////////////////////////////////////////////
 int Posix_File::seek(int64_t offset)
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
 Posix_File::Posix_File(const char *file_name, Open_Mode mode)
 /////////////////////////////////////////////////////////////////////////////
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
  }
  else if (mode == Open_Mode::create_new)
   fd = open(file_name, O_RDWR | O_CREAT | O_EXCL, 00644);
  else if (mode == Open_Mode::write_existing)
   fd = open(file_name, O_RDWR);
  else
   fd = open(file_name, O_RDONLY);

  if (fd < 0)
   throw Exception("Could not create file: " + std::string(file_name));

  if (mode != Open_Mode::read_existing && !lock_file())
   throw Exception("File locked: " + std::string(file_name));

  this->mode = mode;
 }

 /////////////////////////////////////////////////////////////////////////////
 int64_t Posix_File::get_size() const
 /////////////////////////////////////////////////////////////////////////////
 {
  struct stat s;

  if (fstat(fd, &s) == 0)
   return int64_t(s.st_size);
  else
   return 0;
 }

 /////////////////////////////////////////////////////////////////////////////
 Posix_File::~Posix_File()
 /////////////////////////////////////////////////////////////////////////////
 {
  flush();
  close(fd);
 }
}
