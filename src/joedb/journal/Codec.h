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
   virtual std::string encode(const char *buffer, size_t size)
   {
    return std::string(buffer, size);
   }

   virtual std::string decode(const std::string &encoded)
   {
    return encoded;
   }

   virtual ~Codec() = default;
 };
}

#endif
