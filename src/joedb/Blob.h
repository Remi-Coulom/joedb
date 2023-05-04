#ifndef joedb_Blob_declared
#define joedb_Blob_declared

#include <stdint.h>
#include <stddef.h>
#include <string>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Blob
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   int64_t position;
   size_t size;

  public:
   Blob(int64_t position, size_t size):
    position(position),
    size(size)
   {
   }

   Blob(): Blob(0, 0)
   {
   }

   bool is_null() const noexcept {return position == 0;}
   int64_t get_checkpoin() const noexcept {return position;}
   size_t get_size() const noexcept {return size;}
 };
}

#endif
