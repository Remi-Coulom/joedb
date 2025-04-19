#ifndef joedb_Abstract_File_declared
#define joedb_Abstract_File_declared

#include <stddef.h>
#include <stdint.h>

namespace joedb
{
 /// @ingroup journal
 class Abstract_File
 {
  public:
   /// Get the size of the file, or -1 if it is unknown
   virtual int64_t get_size() const {return -1;}

   /// Read a range of bytes
   ///
   /// The returned value may be less than size, even if the end of the file
   /// is not reached. 0 is returned if the end of the file is reached.
   virtual size_t pread(char *data, size_t size, int64_t offset) const {return 0;}
   virtual void pwrite(const char *data, size_t size, int64_t offset) {}

   virtual void sync() {}

   virtual void shared_lock(int64_t start, int64_t size) {}
   virtual void exclusive_lock(int64_t start, int64_t size) {}
   virtual void unlock(int64_t start, int64_t size) noexcept {}

   virtual ~Abstract_File() = default;
 };
}

#endif
