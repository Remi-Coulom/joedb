#ifndef joedb_File_View_declared
#define joedb_File_View_declared

#include "joedb/journal/Buffered_File.h"

namespace joedb
{
 /// @ingroup journal
 class File_View: public Buffered_File
 {
  private:
   Abstract_File &file;

  public:
   File_View(Buffered_File &file): Buffered_File(file.get_mode()), file(file)
   {
   }

   int64_t get_size() const override
   {
    return file.get_size();
   }

   size_t pread(char *data, size_t size, int64_t offset) const override
   {
    return file.pread(data, size, offset);
   }

   void pwrite(const char *data, size_t size, int64_t offset) override
   {
    return file.pwrite(data, size, offset);
   }

   void sync() override
   {
    file.sync();
   }

   void datasync() override
   {
    file.datasync();
   }

   void shared_lock(int64_t start, int64_t size) override
   {
    file.shared_lock(start, size);
   }

   void exclusive_lock(int64_t start, int64_t size) override
   {
    file.exclusive_lock(start, size);
   }

   void unlock(int64_t start, int64_t size) noexcept override
   {
    file.unlock(start, size);
   }
 };
}

#endif
