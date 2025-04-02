#ifndef joedb_Brotli_Decoder_declared
#define joedb_Brotli_Decoder_declared

#include "joedb/journal/Decoder.h"

namespace joedb
{
 /// \ingroup journal
 class Brotli_Decoder: public virtual Decoder
 {
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
