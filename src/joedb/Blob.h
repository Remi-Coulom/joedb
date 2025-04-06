#ifndef joedb_Blob_declared
#define joedb_Blob_declared

#include <stdint.h>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Blob
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   int64_t position;

  public:
   explicit Blob(int64_t position):
    position(position)
   {
   }

   Blob(): Blob(0)
   {
   }

   bool is_null() const {return position == 0;}

   int64_t get_position() const noexcept {return position;}
 };
}

#endif
