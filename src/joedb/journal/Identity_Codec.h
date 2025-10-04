#ifndef joedb_Identity_Codec_declared
#define joedb_Identity_Codec_declared

#include "joedb/journal/Codec.h"
#include "joedb/error/assert.h"

#include <cstring>

namespace joedb
{
 /// @ingroup journal
 class Identity_Codec: public Codec
 {
  public:
   std::string encode(std::string_view decoded) override
   {
    return std::string(decoded);
   }

   void decode
   (
    std::string_view encoded,
    char *decoded,
    size_t decoded_size
   ) override
   {
    JOEDB_DEBUG_ASSERT(encoded.size() == decoded_size);
    std::memcpy(decoded, encoded.data(), decoded_size);
   }
 };
}

#endif
