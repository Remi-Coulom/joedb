#ifndef joedb_Brotli_Codec_declared
#define joedb_Brotli_Codec_declared

#include "joedb/journal/Brotli_Decoder.h"
#include "joedb/journal/Codec.h"

namespace joedb
{
 /// \ingroup journal
 class Brotli_Codec: public Codec, public Brotli_Decoder
 {
  public:
   std::string encode
   (
    const char *decoded,
    size_t decoded_size
   )
   override;
 };
}

#endif
