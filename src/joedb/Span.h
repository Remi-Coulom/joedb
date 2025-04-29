#ifndef joedb_Span_declared
#define joedb_Span_declared

#include "joedb/error/assert.h"

#include <stddef.h>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 template<typename T> class Span
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   T *p;
   size_t size;

  public:
   Span(T *p, size_t size): p(p), size(size)
   {
   }

   T &operator[](size_t i)
   {
    JOEDB_DEBUG_ASSERT(i < size);
    return p[i];
   }

   const T &operator[](size_t i) const
   {
    JOEDB_DEBUG_ASSERT(i < size);
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
