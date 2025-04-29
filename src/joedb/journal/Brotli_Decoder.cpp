#include "joedb/journal/Brotli_Decoder.h"
#include "joedb/error/Exception.h"
#include "joedb/error/assert.h"

#include <brotli/decode.h>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void Brotli_Decoder::decode
 ////////////////////////////////////////////////////////////////////////////
 (
  const std::string &encoded,
  char *decoded,
  size_t decoded_size
 )
 {
  size_t brotli_decoded_size = decoded_size;

  const auto result = BrotliDecoderDecompress
  (
   encoded.size(),
   (const uint8_t *)(encoded.data()),
   &brotli_decoded_size,
   (uint8_t *)decoded
  );

  JOEDB_DEBUG_ASSERT(brotli_decoded_size == decoded_size);

  if (result != BROTLI_DECODER_RESULT_SUCCESS)
   throw Exception("Brotli decompression failed");
 }
}
