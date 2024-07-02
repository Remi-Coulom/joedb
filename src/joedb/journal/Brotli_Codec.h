#ifndef joedb_Brotli_Codec_declared
#define joedb_Brotli_Codec_declared

#ifdef JOEDB_HAS_BROTLI

#include "joedb/journal/Codec.h"

#include <brotli/encode.h>
#include <brotli/decode.h>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Brotli_Codec: public Codec
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   virtual std::string encode(const char *buffer, size_t size)
   {
    return std::string(buffer, size);
   }

   virtual std::string decode(const std::string &encoded)
   {
    return encoded;
   }
 };
}

#endif

#endif
