#ifndef joedb_Codec_declared
#define joedb_Codec_declared

#include <string>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Codec
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   virtual std::string encode(const char *decoded, size_t decoded_size) = 0;

   virtual void decode
   (
    const std::string &encoded,
    char *decoded,
    size_t decoded_size
   ) = 0;

   virtual ~Codec() = default;
 };
}

#endif
