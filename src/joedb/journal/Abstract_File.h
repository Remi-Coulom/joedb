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
  // Should size be int64_t instead of size_t?
  // be careful when sizeof(size_t) == 4

  private:
   virtual size_t raw_read(char *data, size_t size)
   {
    return raw_pread(data, size, file_position);
   }

   virtual void raw_write(const char *data, size_t size)
   {
    raw_pwrite(data, size, file_position);
   }

   virtual size_t raw_pread(char *data, size_t size, int64_t offset)
   {
    raw_seek(offset);
    return raw_read(data, size);
   }

   virtual void raw_pwrite(const char *data, size_t size, int64_t offset)
   {
    raw_seek(offset);
    raw_write(data, size);
   }

   virtual void raw_seek(int64_t offset) = 0; // 0 = OK, 1 = error

   int64_t file_position;

  protected:
   int64_t slice_start;
   int64_t slice_length;

   int64_t get_file_position() const {return file_position;}

   size_t pos_read(char *data, size_t size)
   {
    size_t result = raw_read(data, size);
    file_position += result;
    return result;
   }

   void pos_write(const char *data, size_t size)
   {
    raw_write(data, size);
    file_position += size;
   }

   void seek(int64_t offset)
   {
    raw_seek(slice_start + offset);
    file_position = offset;
   }

   // -1 means "unknown"
   virtual int64_t raw_get_size() const {return -1;}
   virtual void raw_sync() {}

   // No need to use slice_start for lock functions:
   //  - do not support writing multiple slices simultaneously
   //  - public functions are head and tail only
   virtual void shared_lock(int64_t start, int64_t size) {}
   virtual void exclusive_lock(int64_t start, int64_t size) {}
   virtual void unlock(int64_t start, int64_t size) {}

  public:
   Abstract_File()
   {
    slice_start = 0;
    slice_length = 0;
    file_position = 0;
   }

   // Note: file_position is undefined after those

   size_t pos_pread(char *data, size_t size, int64_t offset)
   {
    return raw_pread(data, size, slice_start + offset);
   }

   void pos_pwrite(const char *data, size_t size, int64_t offset)
   {
    raw_pwrite(data, size, slice_start + offset);
   }

   int64_t get_size() const
   {
    if (slice_start + slice_length)
     return slice_length;
    else
     return raw_get_size();
   }

   virtual ~Abstract_File() = default;
 };
}

#endif
