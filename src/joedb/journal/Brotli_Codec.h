#ifndef joedb_Brotli_Codec_declared
#define joedb_Brotli_Codec_declared

#ifdef JOEDB_HAS_BROTLI

#include "joedb/journal/Codec.h"
#include "joedb/Exception.h"

#include <brotli/encode.h>
#include <brotli/decode.h>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Brotli_Decoder
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   BrotliDecoderState * const state;

  public:
   Brotli_Decoder():
    state(BrotliDecoderCreateInstance(nullptr, nullptr, nullptr))
   {
    if (state == nullptr)
     throw joedb::Runtime_Error("could not allocate Brotli decoder");
   }

   void decode
   (
    const std::string &encoded,
    char *decoded,
    const size_t decoded_size
   )
   {
    size_t brotli_decoded_size = decoded_size;

    BrotliDecoderDecompress
    (
     encoded.size(),
     (const uint8_t *)(encoded.data()),
     &brotli_decoded_size,
     (uint8_t *)decoded
    );

    JOEDB_ASSERT(brotli_decoded_size == decoded_size);
   }

   ~Brotli_Decoder()
   {
    BrotliDecoderDestroyInstance(state);
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Brotli_Encoder
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   const int quality;
   const int lgwin;
   BrotliEncoderMode mode;

   BrotliEncoderState * const state;

  public:
   Brotli_Encoder
   (
    int quality = BROTLI_DEFAULT_QUALITY,
    int lgwin = BROTLI_DEFAULT_WINDOW,
    BrotliEncoderMode mode = BROTLI_DEFAULT_MODE
   ):
    quality(quality),
    lgwin(lgwin),
    mode(mode),
    state(BrotliEncoderCreateInstance(nullptr, nullptr, nullptr))
   {
    if (state == nullptr)
     throw joedb::Runtime_Error("could not allocate Brotli encoder");
   }

   std::string encode(const char *buffer, size_t size)
   {
    std::string encoded(BrotliEncoderMaxCompressedSize(size), 0);

    size_t encoded_size;

    BrotliEncoderCompress
    (
     quality,
     lgwin,
     mode,
     size,
     (const uint8_t *)buffer,
     &encoded_size,
     (uint8_t *)encoded.data()
    );

    encoded.resize(encoded_size);

    return encoded;
   }

   ~Brotli_Encoder()
   {
    BrotliEncoderDestroyInstance(state);
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Brotli_Codec: public Codec
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Brotli_Encoder encoder;
   Brotli_Decoder decoder;

  public:
   std::string encode(const char *decoded, size_t decoded_size) override
   {
    return encoder.encode(decoded, decoded_size);
   }

   void decode
   (
    const std::string &encoded,
    char *decoded,
    size_t decoded_size
   )
   override
   {
    decoder.decode(encoded, decoded, decoded_size);
   }
 };
}

#endif

#endif
