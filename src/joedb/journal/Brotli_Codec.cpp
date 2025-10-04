#include "joedb/journal/Brotli_Codec.h"
#include "joedb/error/Exception.h"

#include <brotli/encode.h>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 std::string Brotli_Codec::encode(std::string_view decoded)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::string encoded(BrotliEncoderMaxCompressedSize(decoded.size()), 0);

  size_t encoded_size = encoded.size();

  const auto result = BrotliEncoderCompress
  (
   BROTLI_DEFAULT_QUALITY,
   BROTLI_DEFAULT_WINDOW,
   BROTLI_DEFAULT_MODE,
   decoded.size(),
   (const uint8_t *)decoded.data(),
   &encoded_size,
   (uint8_t *)encoded.data()
  );

  if (result != BROTLI_TRUE)
   throw Exception("Brotli compression failed");

  encoded.resize(encoded_size);

  return encoded;
 }
}
