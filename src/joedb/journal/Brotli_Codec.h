#ifndef joedb_Brotli_Codec_declared
#define joedb_Brotli_Codec_declared

#ifdef JOEDB_HAS_BROTLI

#include "joedb/journal/Codec.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Brotli_Codec: public Codec
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   std::string encode
   (
    const char *decoded,
    size_t decoded_size
   )
   override;

   void decode
   (
    const std::string &encoded,
    char *decoded,
    size_t decoded_size
   )
   override;
 };
}

#endif

#endif
