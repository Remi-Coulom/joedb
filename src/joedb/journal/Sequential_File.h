#ifndef joedb_Sequential_File_declared
#define joedb_Sequential_File_declared

#include "joedb/journal/Abstract_File.h"

namespace joedb
{
 /// \ingroup journal
 class Sequential_File: public Abstract_File
 {
  private:
   int64_t position = 0;

  public:
   void sequential_seek(int64_t new_position)
   {
    position = new_position;
   }

   size_t sequential_read(char *data, size_t size)
   {
    const size_t result = pread(data, size, position);
    position += result;
    return result;
   }

   void sequential_write(const char *data, size_t size)
   {
    pwrite(data, size, position);
    position += size;
   }

   int64_t get_position() const {return position;}
 };
}

#endif
