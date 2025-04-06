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

 ////////////////////////////////////////////////////////////////////////////
 class Blob_Reader
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   virtual std::string read_blob_data(Blob blob) const = 0;
   virtual ~Blob_Reader();
 };
}

#endif
