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
  private:
   virtual void raw_seek(int64_t offset);
   virtual size_t raw_read(char *data, size_t size);
   virtual void raw_write(const char *data, size_t size);

   // Implement either raw_seek + raw_read + raw_write or pread + pwrite
   // Default pread/pwrite do a raw_seek + raw_read/raw_write

  protected:
   virtual void raw_sync();
   virtual void shared_lock(int64_t start, int64_t size);
   virtual void exclusive_lock(int64_t start, int64_t size);
   virtual void unlock(int64_t start, int64_t size);

  public:
   virtual size_t pread(char *data, size_t size, int64_t offset);
   virtual void pwrite(const char *data, size_t size, int64_t offset);
   virtual int64_t get_size() const; // -1 means "unknown"

   virtual ~Abstract_File();
 };
}

#endif
