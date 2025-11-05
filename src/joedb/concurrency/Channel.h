#ifndef joedb_Channel_declared
#define joedb_Channel_declared

#include <stddef.h>

namespace joedb
{
 /// @ingroup concurrency
 class Channel
 {
  public:
   virtual size_t write_some(const char *data, size_t size) = 0;
   virtual size_t read_some(char *data, size_t size) = 0;

   void write(const char *data, size_t size);
   void read(char *data, size_t size);

   virtual ~Channel();
 };
}

#endif
