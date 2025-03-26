#ifndef joedb_Abstract_File_declared
#define joedb_Abstract_File_declared

#include <stddef.h>
#include <stdint.h>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Abstract_File
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   virtual int64_t get_size() const {return -1;}
   virtual size_t pread(char *data, size_t size, int64_t offset) const {return 0;}
   virtual void pwrite(const char *data, size_t size, int64_t offset) {}

   virtual void sync() {}

   virtual void shared_lock(int64_t start, int64_t size) {}
   virtual void exclusive_lock(int64_t start, int64_t size) {}
   virtual void unlock(int64_t start, int64_t size) {}

   virtual ~Abstract_File() = default;
 };
}

#endif
