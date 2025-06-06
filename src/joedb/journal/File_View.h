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
 };
}

#endif
