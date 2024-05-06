#include "Abstract_File.h"

namespace joedb
{
 void Abstract_File::raw_seek(int64_t offset) {}

 size_t Abstract_File::raw_read(char *data, size_t size)
 {
  return pread(data, size, file_position);
 }

 void Abstract_File::raw_write(const char *data, size_t size)
 {
  pwrite(data, size, file_position);
 }

 size_t Abstract_File::pread(char *data, size_t size, int64_t offset)
 {
  raw_seek(offset);
  return raw_read(data, size);
 }

 void Abstract_File::pwrite(const char *data, size_t size, int64_t offset)
 {
  raw_seek(offset);
  raw_write(data, size);
 }

 int64_t Abstract_File::get_size() const {return -1;}
 void Abstract_File::raw_sync() {}
 void Abstract_File::shared_lock(int64_t start, int64_t size) {}
 void Abstract_File::exclusive_lock(int64_t start, int64_t size) {}
 void Abstract_File::unlock(int64_t start, int64_t size) {}

 Abstract_File::Abstract_File(): file_position(0)
 {
 }

 Abstract_File::~Abstract_File() = default;
}
