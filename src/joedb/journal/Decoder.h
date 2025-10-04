#ifndef joedb_Decoder_declared
#define joedb_Decoder_declared

#include <string_view>

namespace joedb
{
 /// @ingroup journal
 class Decoder
 {
  public:
   virtual void decode
   (
    const std::string_view encoded,
    char *decoded,
    size_t decoded_size
   ) = 0;

   virtual ~Decoder() = default;
 };
}

#endif
