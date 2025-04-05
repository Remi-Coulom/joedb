#ifndef joedb_Identity_Codec_declared
#define joedb_Identity_Codec_declared

#include "joedb/journal/Codec.h"

#include <algorithm>
#include "joedb/error/assert.h"

namespace joedb
{
 /// @ingroup journal
 class Identity_Codec: public Codec
 {
  public:
   std::string encode(const char *decoded, size_t decoded_size) override
   {
    return std::string(decoded, decoded_size);
   }

   void decode
   (
    const std::string &encoded,
    char *decoded,
    size_t decoded_size
   ) override
   {
    JOEDB_ASSERT(encoded.size() == decoded_size);
    std::copy_n(encoded.data(), decoded_size, decoded);
   }
 };
}

#endif
