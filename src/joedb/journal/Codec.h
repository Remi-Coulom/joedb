#ifndef joedb_Codec_declared
#define joedb_Codec_declared

#include "joedb/journal/Decoder.h"

#include <string>

namespace joedb
{
 /// @ingroup journal
 class Codec: public virtual Decoder
 {
  public:
   virtual std::string encode
   (
    const char *decoded,
    size_t decoded_size
   ) = 0;
 };
}

#endif
