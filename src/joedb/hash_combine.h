#ifndef joedb_hash_combine_declared
#define joedb_hash_combine_declared

#include <stddef.h>

namespace joedb
{
 inline size_t hash_combine(size_t h1, size_t h2)
 {
  return h1 + 0x9e3779b9 + (h2 << 6) + (h2 >> 2);
 }
}

#endif

