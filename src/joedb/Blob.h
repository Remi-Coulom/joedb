#ifndef joedb_Blob_declared
#define joedb_Blob_declared

#include <stdint.h>
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
   Blob(int64_t position, int64_t size):
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

 static_assert(sizeof(Blob) == 2 * sizeof(int64_t), "Bad Blob size");

 ////////////////////////////////////////////////////////////////////////////
 class Blob_Reader
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   virtual std::string read_blob_data(Blob blob) = 0;
   virtual ~Blob_Reader() = default;
 };

 ////////////////////////////////////////////////////////////////////////////
 class Blob_Writer
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   virtual Blob write_blob_data(const std::string &data) {return Blob();}
   virtual ~Blob_Writer() = default;
 };
}

#endif
