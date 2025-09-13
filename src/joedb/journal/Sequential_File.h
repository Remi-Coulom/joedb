#ifndef joedb_Sequential_File_declared
#define joedb_Sequential_File_declared

#include "joedb/journal/Abstract_File.h"

namespace joedb
{
 /// @ingroup journal
 class Sequential_File // TODO -> File_Cursor
 {
  private:
   int64_t position = 0;

  protected:
   Abstract_File &file;

  public:
   Sequential_File(Abstract_File &file): file(file) {}

   void sequential_seek(int64_t new_position) noexcept
   {
    position = new_position;
   }

   size_t sequential_read(char *data, size_t size)
   {
    const size_t result = file.pread(data, size, position);
    position += result;
    return result;
   }

   void sequential_write(const char *data, size_t size)
   {
    file.pwrite(data, size, position);
    position += size;
   }

   int64_t get_position() const noexcept {return position;}
 };
}

#endif
