#ifndef joedb_File_View_declared
#define joedb_File_View_declared

#include "joedb/journal/Buffered_File.h"

namespace joedb
{
 /// @ingroup journal
 class File_View: public Buffered_File
 {
  private:
   const Abstract_File &file;

  public:
   File_View(const Abstract_File &file):
    Buffered_File(Open_Mode::read_existing),
    file(file)
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
 };
}

#endif
