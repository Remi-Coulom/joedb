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

   // Implement either seek + read + write or pread + pwrite

   int64_t file_position;

  protected:
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
    raw_seek(offset);
    file_position = offset;
   }

   virtual void raw_sync();

   virtual void shared_lock(int64_t start, int64_t size);
   virtual void exclusive_lock(int64_t start, int64_t size);
   virtual void unlock(int64_t start, int64_t size);

  public:
   Abstract_File();

   // Note: file_position is undefined after those

   virtual size_t pread(char *data, size_t size, int64_t offset);
   virtual void pwrite(const char *data, size_t size, int64_t offset);

   // -1 means "unknown"
   virtual int64_t get_size() const;

   virtual ~Abstract_File();
 };
}

#endif
