#include "Abstract_File.h"

namespace joedb
{
 void Abstract_File::raw_seek(int64_t offset) {}
 size_t Abstract_File::raw_read(char *data, size_t size) {return 0;}
 void Abstract_File::raw_write(const char *data, size_t size) {}

 void Abstract_File::raw_sync() {}
 void Abstract_File::shared_lock(int64_t start, int64_t size) {}
 void Abstract_File::exclusive_lock(int64_t start, int64_t size) {}
 void Abstract_File::unlock(int64_t start, int64_t size) {}

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

 Abstract_File::~Abstract_File() = default;
}
