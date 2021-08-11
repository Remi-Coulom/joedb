#include "joedb/journal/SHA_256.h"

#include <iostream>
#include <iomanip>

/////////////////////////////////////////////////////////////////////////////
int main()
/////////////////////////////////////////////////////////////////////////////
{
 const int chunk_size = 64;
 char buffer[chunk_size];
 int64_t size = 0;

 auto &rdbuf = *std::cin.rdbuf();

 joedb::SHA_256 sha_256;

 while (true)
 {
  int64_t read = int64_t(rdbuf.sgetn(buffer, chunk_size));
  size += read;
  if (read < chunk_size)
  {
   sha_256.process_final_chunk(buffer, size);
   break;
  }
  else
   sha_256.process_chunk(buffer);
 }

 const auto &h = sha_256.get_hash();

 std::cout << std::hex;
 std::cout << std::setfill('0');
 for (int i = 0; i < 8; i++)
  std::cout << std::setw(8) << h[i];
 std::cout << '\n';

 return 0;
}
