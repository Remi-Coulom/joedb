#ifndef joedb_Codec_declared
#define joedb_Codec_declared

#include <string>
#include <algorithm>

#include "joedb/assert.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Codec
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   virtual std::string encode(const char *decoded, size_t decoded_size)
   {
    return std::string(decoded, decoded_size);
   }

   virtual void decode
   (
    const std::string &encoded,
    char *decoded,
    size_t decoded_size
   )
   {
    JOEDB_ASSERT(encoded.size() <= decoded_size);
    std::copy_n(encoded.data(), encoded.size(), decoded);
   }

   virtual ~Codec() = default;
 };
}

#endif
