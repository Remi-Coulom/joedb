#include "joedb/journal/Brotli_Codec.h"
#include "joedb/error/Exception.h"
#include "joedb/error/assert.h"

#include <brotli/encode.h>
#include <brotli/decode.h>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 std::string Brotli_Codec::encode(const char *decoded, size_t decoded_size)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::string encoded(BrotliEncoderMaxCompressedSize(decoded_size), 0);

  size_t encoded_size = encoded.size();

  const auto result = BrotliEncoderCompress
  (
   BROTLI_DEFAULT_QUALITY,
   BROTLI_DEFAULT_WINDOW,
   BROTLI_DEFAULT_MODE,
   decoded_size,
   (const uint8_t *)decoded,
   &encoded_size,
   (uint8_t *)encoded.data()
  );

  if (result != BROTLI_TRUE)
   throw Exception("Brotli compression failed");

  encoded.resize(encoded_size);

  return encoded;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Brotli_Codec::decode
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

  JOEDB_ASSERT(brotli_decoded_size == decoded_size);

  if (result != BROTLI_DECODER_RESULT_SUCCESS)
   throw Exception("Brotli decompression failed");
 }
}
