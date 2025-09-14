#ifndef joedb_File_Iterator_declared
#define joedb_File_Iterator_declared

#include "joedb/journal/Abstract_File.h"

namespace joedb
{
 /// @ingroup journal
 class File_Iterator
 {
  private:
   int64_t position = 0;

  protected:
   Abstract_File &file;

  public:
   File_Iterator(Abstract_File &file): file(file) {}

   void seek(int64_t new_position) noexcept
   {
    position = new_position;
   }

   int64_t get_position() const noexcept {return position;}

   size_t read(char *data, size_t size)
   {
    const size_t result = file.pread(data, size, position);
    position += result;
    return result;
   }

   void write(const char *data, size_t size)
   {
    file.pwrite(data, size, position);
    position += size;
   }
 };
}

#endif
