#include "joedb/ui/base64.h"

#include <stdint.h>

namespace joedb::ui
{
 static constexpr char base64_decoding[256] =
 {
   0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,

   0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,

   0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0, 62,  0,  0,  0, 63,

  52, 53, 54, 55, 56, 57, 58, 59,
  60, 61,  0,  0,  0,  0,  0,  0,

   0,  0,  1,  2,  3,  4,  5,  6,
   7,  8,  9, 10, 11, 12, 13, 14,

  15, 16, 17, 18, 19, 20, 21, 22,
  23, 24, 25,  0,  0,  0,  0,  0,

   0, 26, 27, 28, 29, 30, 31, 32,
  33, 34, 35, 36, 37, 38, 39, 40,

  41, 42, 43, 44, 45, 46, 47, 48,
  49, 50, 51,  0,  0,  0,  0,  0,

   0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,

   0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,

   0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,

   0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,

   0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,

   0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,

   0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,

   0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,
 };

 /////////////////////////////////////////////////////////////////////////////
 std::string base64_decode(const std::string &input)
 /////////////////////////////////////////////////////////////////////////////
 {
  std::string result;

  size_t N = input.size() / 4;
  size_t remainder = 0;

  if (N > 0)
  {
   if (input[input.size() - 1] == '=')
   {
    N--;
    if (input[input.size() - 2] == '=')
     remainder = 1;
    else
     remainder = 2;
   }
  }

  result.resize(3 * N + remainder);

  for (size_t i = 0; i < N; i++)
  {
   const uint32_t word =
    (uint32_t(base64_decoding[uint8_t(input[4 * i + 0])]) << 18) |
    (uint32_t(base64_decoding[uint8_t(input[4 * i + 1])]) << 12) |
    (uint32_t(base64_decoding[uint8_t(input[4 * i + 2])]) <<  6) |
    (uint32_t(base64_decoding[uint8_t(input[4 * i + 3])])      );

   result[3 * i + 0] = char(word >> 16);
   result[3 * i + 1] = char(word >>  8);
   result[3 * i + 2] = char(word      );
  }

  if (remainder == 2)
  {
   const uint32_t word =
    (uint32_t(base64_decoding[uint8_t(input[4 * N + 0])]) << 12) |
    (uint32_t(base64_decoding[uint8_t(input[4 * N + 1])]) <<  6) |
    (uint32_t(base64_decoding[uint8_t(input[4 * N + 2])])      );

   result[3 * N + 0] = char(word >> 10);
   result[3 * N + 1] = char(word >>  2);
  }
  else if (remainder == 1)
  {
   const uint32_t word =
    (uint32_t(base64_decoding[uint8_t(input[4 * N + 0])]) <<  6) |
    (uint32_t(base64_decoding[uint8_t(input[4 * N + 1])])      );

   result[3 * N + 0] = char(word >>  4);
  }

  return result;
 }
}
