#ifndef joedb_SHA_256_declared
#define joedb_SHA_256_declared

#include <array>
#include <stdint.h>
#include <algorithm>

// https://en.wikipedia.org/wiki/SHA-2

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 constexpr uint32_t rotr(uint32_t x, uint8_t n)
 ////////////////////////////////////////////////////////////////////////////
 {
  return (x >> n) | (x << ((-n) & 31));
 }

 ////////////////////////////////////////////////////////////////////////////
 class SHA_256
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   static constexpr std::array<uint32_t, 8> hash_init
   {
    {
     0x6a09e667,
     0xbb67ae85,
     0x3c6ef372,
     0xa54ff53a,
     0x510e527f,
     0x9b05688c,
     0x1f83d9ab,
     0x5be0cd19
    }
   };

   static constexpr std::array<uint32_t, 64> k
   {
    {
     0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
     0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
     0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
     0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
     0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
     0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
     0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
     0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
     0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
     0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
     0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
     0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
     0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
     0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
     0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
     0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    }
   };

  public:
   typedef std::array<uint32_t, 8> Hash;

  private:
   Hash hash;

  public:
   SHA_256(): hash(hash_init) {}
   const Hash &get_hash() const {return hash;}
   static constexpr size_t chunk_size = 64;

   /////////////////////////////////////////////////////////////////////////
   void process_chunk(const char *data)
   /////////////////////////////////////////////////////////////////////////
   {
    // process 512 bits (32 * 16, 8 * 64) of data

    std::array<uint32_t, 64> w;

    {
     const uint8_t *u8_data = reinterpret_cast<const uint8_t *>(data);

     for (uint32_t i = 0; i < 16; i++)
      w[i] =
      (uint32_t(u8_data[4 * i + 0]) << 24) |
      (uint32_t(u8_data[4 * i + 1]) << 16) |
      (uint32_t(u8_data[4 * i + 2]) <<  8) |
      (uint32_t(u8_data[4 * i + 3])      );
    }

    for (uint32_t i = 16; i < 64; i++)
    {
     const uint32_t w0 = w[i - 15];
     const uint32_t s0 = rotr(w0, 7) ^ rotr(w0, 18) ^ (w0 >> 3);
     const uint32_t w1 = w[i - 2];
     const uint32_t s1 = rotr(w1, 17) ^ rotr(w1, 19) ^ (w1 >> 10);

     w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }

    Hash x(hash);

    for (uint32_t i = 0; i < 64; i++)
    {
     const uint32_t S1 = rotr(x[4], 6) ^ rotr(x[4], 11) ^ rotr(x[4], 25);
     const uint32_t ch = (x[4] & x[5]) ^ ((~x[4]) & x[6]);
     const uint32_t temp1 = x[7] + S1 + ch + k[i] + w[i];
     const uint32_t S0 = rotr(x[0], 2) ^ rotr(x[0], 13) ^ rotr(x[0], 22);
     const uint32_t maj = (x[0] & x[1]) ^ (x[0] & x[2]) ^ (x[1] & x[2]);
     const uint32_t temp2 = S0 + maj;

     x[7] = x[6];
     x[6] = x[5];
     x[5] = x[4];
     x[4] = x[3] + temp1;
     x[3] = x[2];
     x[2] = x[1];
     x[1] = x[0];
     x[0] = temp1 + temp2;
    };

    for (uint32_t i = 0; i < 8; i++)
     hash[i] += x[i];
   }

   /////////////////////////////////////////////////////////////////////////
   void process_final_chunk
   /////////////////////////////////////////////////////////////////////////
   (
    const char * const data,
    const uint64_t total_length_in_bytes
   )
   {
    // data points to the final n bytes, 0 <= n < 64
    std::array<uint32_t, 32> final_chunks{};
    uint8_t *byte_buffer = reinterpret_cast<uint8_t *>(&final_chunks[0]);
    uint32_t n = uint32_t(total_length_in_bytes & 0x3fULL);
    std::copy_n(data, n, byte_buffer);
    byte_buffer[n] = 0x80;

    const int chunk_count = n + 9 <= 64 ? 1 : 2;

    {
     uint64_t length_in_bits = total_length_in_bytes * 8;
     for (int index = chunk_count * 64, i = 8; --index, --i >= 0;)
     {
      byte_buffer[index] = uint8_t(length_in_bits);
      length_in_bits >>= 8;
     }
    }

    for (uint32_t i = 0; i < uint32_t(chunk_count); i++)
     process_chunk(reinterpret_cast<char *>(&final_chunks[16 * i]));
   }
 };
}

#endif
