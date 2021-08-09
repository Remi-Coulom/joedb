#ifndef joedb_Range_declared
#define joedb_Range_declared

#include "joedb/assert.h"

#include <stddef.h>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 template<typename T> class Range
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   T *p;
   size_t size;

  public:
   Range(T *p, size_t size): p(p), size(size)
   {
   }

   T &operator[](size_t i)
   {
    JOEDB_ASSERT(i < size);
    return p[i];
   }

   const T &operator[](size_t i) const
   {
    JOEDB_ASSERT(i < size);
    return p[i];
   }

   size_t get_size() const
   {
    return size;
   }

   T *begin()
   {
    return p;
   }

   T *end()
   {
    return p + size;
   }

   const T *begin() const
   {
    return p;
   }

   const T *end() const
   {
    return p + size;
   }
 };
}

#endif
