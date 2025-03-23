#ifndef joedb_Position_File_declared
#define joedb_Position_File_declared

#include "joedb/journal/Abstract_File.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Position_File: public Abstract_File
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   int64_t position;

  public:
   Position_File(): position(0) {}

   void pos_seek(int64_t new_position)
   {
    position = new_position;
   }

   size_t pos_read(char *data, size_t size)
   {
    const size_t result = pread(data, size, position);
    position += result;
    return result;
   }

   void pos_write(const char *data, size_t size)
   {
    pwrite(data, size, position);
    position += size;
   }

   int64_t get_position() const {return position;}
 };
}

#endif
