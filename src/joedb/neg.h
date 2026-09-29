#ifndef joedb_neg_defined
#define joedb_neg_defined

#include "stdint.h"

namespace joedb
{
 inline int64_t neg(int64_t x)
 {
  return int64_t(-uint64_t(x)); // avoid UB
 }
}

#endif
