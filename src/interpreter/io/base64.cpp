#include "base64.h"
#include "stdint.h"

/////////////////////////////////////////////////////////////////////////////
std::string joedb::base64_encode(const std::string &input)
/////////////////////////////////////////////////////////////////////////////
{
 static char const * const base64_codes =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

 const size_t N = input.size() / 3;
 const size_t remainder = input.size() % 3;

 std::string result;
 result.resize(4 * (N + (remainder != 0)));

 for (size_t i = 0; i < N; i++)
 {
  int32_t word = (uint8_t(input[3 * i + 0]) << 16) |
                 (uint8_t(input[3 * i + 1]) <<  8) |
                 (uint8_t(input[3 * i + 2])      );
  result[4 * i + 0] = base64_codes[(word >> 18)       ];
  result[4 * i + 1] = base64_codes[(word >> 12) & 0x3f];
  result[4 * i + 2] = base64_codes[(word >>  6) & 0x3f];
  result[4 * i + 3] = base64_codes[(word      ) & 0x3f];
 }

 if (remainder == 2)
 {
  int32_t word = (uint8_t(input[3 * N + 0]) << 10) |
                 (uint8_t(input[3 * N + 1]) <<  2);
  result[4 * N + 0] = base64_codes[(word >> 12)       ];
  result[4 * N + 1] = base64_codes[(word >>  6) & 0x3f];
  result[4 * N + 2] = base64_codes[(word      ) & 0x3f];
  result[4 * N + 3] = '=';
 }
 else if (remainder == 1)
 {
  int32_t word = (uint8_t(input[3 * N + 0]) << 4);
  result[4 * N + 0] = base64_codes[(word >>  6)       ];
  result[4 * N + 1] = base64_codes[(word      ) & 0x3f];
  result[4 * N + 2] = '=';
  result[4 * N + 3] = '=';
 }

 return result;
}
