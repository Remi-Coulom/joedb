#include "joedb/journal/Brotli_Codec.h"
#include "joedb/Exception.h"
#include "joedb/assert.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Brotli_Decoder::Brotli_Decoder():
 ////////////////////////////////////////////////////////////////////////////
  state(BrotliDecoderCreateInstance(nullptr, nullptr, nullptr))
 {
  if (state == nullptr)
   throw joedb::Runtime_Error("could not allocate Brotli decoder");
 }

 ////////////////////////////////////////////////////////////////////////////
 void Brotli_Decoder::decode
 ////////////////////////////////////////////////////////////////////////////
 (
  const std::string &encoded,
  char *decoded,
  const size_t decoded_size
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
   throw joedb::Runtime_Error("Brotli decompression failed");
 }

 ////////////////////////////////////////////////////////////////////////////
 Brotli_Decoder::~Brotli_Decoder()
 ////////////////////////////////////////////////////////////////////////////
 {
  BrotliDecoderDestroyInstance(state);
 }

 ////////////////////////////////////////////////////////////////////////////
 Brotli_Encoder::Brotli_Encoder
 ////////////////////////////////////////////////////////////////////////////
 (
  int quality,
  int lgwin,
  BrotliEncoderMode mode
 ):
  quality(quality),
  lgwin(lgwin),
  mode(mode),
  state(BrotliEncoderCreateInstance(nullptr, nullptr, nullptr))
 {
  if (state == nullptr)
   throw joedb::Runtime_Error("could not allocate Brotli encoder");
 }

 ////////////////////////////////////////////////////////////////////////////
 std::string Brotli_Encoder::encode(const char *buffer, size_t size)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::string encoded(BrotliEncoderMaxCompressedSize(size), 0);

  size_t encoded_size = encoded.size();

  const auto result = BrotliEncoderCompress
  (
   quality,
   lgwin,
   mode,
   size,
   (const uint8_t *)buffer,
   &encoded_size,
   (uint8_t *)encoded.data()
  );

  if (result != BROTLI_TRUE)
   throw joedb::Runtime_Error("Brotli compression failed");

  encoded.resize(encoded_size);

  return encoded;
 }

 ////////////////////////////////////////////////////////////////////////////
 Brotli_Encoder::~Brotli_Encoder()
 ////////////////////////////////////////////////////////////////////////////
 {
  BrotliEncoderDestroyInstance(state);
 }

 ////////////////////////////////////////////////////////////////////////////
 std::string Brotli_Codec::encode(const char *decoded, size_t decoded_size)
 ////////////////////////////////////////////////////////////////////////////
 {
  return encoder.encode(decoded, decoded_size);
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
  decoder.decode(encoded, decoded, decoded_size);
 }
}
