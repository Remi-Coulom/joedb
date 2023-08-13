#ifndef joedb_Channel_declared
#define joedb_Channel_declared

#include <stddef.h>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Channel
 ////////////////////////////////////////////////////////////////////////////
 {
  friend class Channel_Lock;

  public:
   virtual size_t write_some(const char *data, size_t size) = 0;
   virtual size_t read_some(char *data, size_t size) = 0;

   void write(const char *data, size_t size)
   {
    size_t n = 0;
    while (n < size)
     n += write_some(data + n, size - n);
   }

   void read(char *data, size_t size)
   {
    size_t n = 0;
    while (n < size)
     n += read_some(data + n, size - n);
   }

   virtual ~Channel();
 };
}

#endif
