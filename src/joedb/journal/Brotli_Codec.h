#ifndef joedb_Brotli_Codec_declared
#define joedb_Brotli_Codec_declared

#ifdef JOEDB_HAS_BROTLI

#include "joedb/journal/Codec.h"

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
   Brotli_Decoder();

   void decode
   (
    const std::string &encoded,
    char *decoded,
    const size_t decoded_size
   );

   ~Brotli_Decoder();
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
   );

   std::string encode(const char *buffer, size_t size);

   ~Brotli_Encoder();
 };

 ////////////////////////////////////////////////////////////////////////////
 class Brotli_Codec: public Codec
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Brotli_Encoder encoder;
   Brotli_Decoder decoder;

  public:
   std::string encode(const char *decoded, size_t decoded_size) override;

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
