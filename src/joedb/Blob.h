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

   int64_t get_position() const noexcept {return position;}
   size_t get_size() const noexcept {return size;}
 };

 ////////////////////////////////////////////////////////////////////////////
 class Blob_Storage
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   virtual std::string read_blob(Blob blob) = 0;
   virtual ~Blob_Storage() = default;
 };
}

#endif
