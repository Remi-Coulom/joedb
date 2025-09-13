#ifndef joedb_Blob_declared
#define joedb_Blob_declared

#include <stdint.h>

namespace joedb
{
 /// @ingroup joedb
 class Blob
 {
  private:
   int64_t position;
   int64_t size;

  public:
   explicit Blob(int64_t position, int64_t size) noexcept:
    position(position),
    size(size)
   {
   }

   Blob() noexcept: Blob(0, 0)
   {
   }

   bool is_null() const noexcept {return position == 0 && size == 0;}
   bool operator<(Blob blob) const noexcept {return position < blob.position;}

   int64_t get_position() const noexcept {return position;}
   int64_t get_size() const noexcept {return size;}
   int64_t get_end() const noexcept {return position + size;}
 };
}

#endif
